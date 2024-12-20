#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <thread>
#include <chrono>
#include "Utils.h"

#define MAX_SIZE 300

const std::string colorGreen = "\x1B[32m";
const std::string colorYellow = "\x1B[33m";
const std::string colorBlue = "\x1B[34m";
const std::string colorReset = "\x1B[0m";
const std::string colorDim = "\x1B[2m";
const std::string colorItalic = "\x1B[3m";

enum LightPatchUnit
{
    R,
    G,
    B,
    Dimmer,
    Strobo,

    R16 = 100,
    G16,
    B16,
    Dimmer16,
    Strobo16
};

#define LIGHT_PATCH_UNIT_SIZE(x) (x >= R16 ? 2 : 1)
#define LIGHT_PATCH_UNIT_TO_STRING(x) (x == R ? "R" : x == G      ? "G"        \
                                                  : x == B        ? "B"        \
                                                  : x == Dimmer   ? "Dimmer"   \
                                                  : x == Strobo   ? "Strobo"   \
                                                  : x == R16      ? "R16"      \
                                                  : x == G16      ? "G16"      \
                                                  : x == B16      ? "B16"      \
                                                  : x == Dimmer16 ? "Dimmer16" \
                                                  : x == Strobo16 ? "Strobo16" \
                                                                  : "Unknown")

struct EffectParams {
    int globalSize = -1, offsetGlobal = 0, localSize = -1, localId = -1, tick;
    void* other;
    EffectParams(int tick = 0, void* other = nullptr): tick(tick), other(other) {}
};
typedef void (*EffectFn)(EffectParams &params, std::vector<std::vector<int>> &values);

struct Light
{
    struct Group
    {
        std::string name;
        std::vector<LightPatchUnit> units;
        std::vector<unsigned int> offsets;
        uint8_t **memory;
        int offset;
        unsigned int size = 0;

        // copy and move constructors
        Group() = default;
        Group(const Group &other) = default;
        Group(Group &&other) = default;

        Group(std::string name, std::vector<LightPatchUnit> units) : name(name), units(units)
        {
            unsigned int counter = 0;
            for (unsigned int i = 0; i < units.size(); i++)
            {
                offsets.push_back(counter);

                LightPatchUnit unit = units[i];
                unsigned int unitSize = LIGHT_PATCH_UNIT_SIZE(unit);

                counter += unitSize;
            }

            size = counter;
        }

        std::vector<int> getValues() const
        {
            std::vector<int> values(units.size());
            for (int i = 0; i < units.size(); i++)
            {
                auto &unit = units[i];
                unsigned int size = LIGHT_PATCH_UNIT_SIZE(unit);
                int value = 0;
                for (int j = 0; j < size; j++)
                {
                    value |= (*memory)[offset + offsets[i] + size - j - 1] << (j * 8);
                }
                values[i] = value;
            }
            return values;
        }

        bool setValues(std::vector<int> values)
        {
            if (values.size() != units.size())
            {
                return false;
            }

            for (int i = 0; i < units.size(); i++)
            {
                if (values[i] == -1)
                    continue;
                auto &unit = units[i];
                unsigned int size = LIGHT_PATCH_UNIT_SIZE(unit);
                for (int j = 0; j < size; j++)
                {
                    (*memory)[offset + offsets[i] + size - j - 1] = (values[i] >> (j * 8)) & 0xFF;
                }
            }

            return true;
        }

        unsigned int getGroupFootprint() const
        {
            return size;
        }

        unsigned int getNumUnits() const
        {
            return units.size();
        }

        std::string describe() const
        {
            std::stringstream ss;
            ss << "\x1B[34m\"" << name << "\"\x1B[0m size: " << size << " {";
            for (int i = 0; i < units.size(); i++)
            {
                if (i != 0)
                    ss << ", ";
                ss << LIGHT_PATCH_UNIT_TO_STRING(units[i]);
            }
            ss << "}";
            return ss.str();
        }

        std::string toString() const
        {
            auto values = getValues();
            std::stringstream ss;
            std::cout << "\x1B[34m\"" << name << "\"\x1B[0m size: " << size << " {";
            for (int i = 0; i < units.size(); i++)
            {
                if (i != 0)
                    ss << ", ";
                ss << LIGHT_PATCH_UNIT_TO_STRING(units[i]) << ": " << colorDim << values[i] << colorReset;
            }
            ss << "}";
            return ss.str();
        }

        friend std::ostream &operator<<(std::ostream &os, const Group &group)
        {
            os << group.toString();
            return os;
        }
    };

    // the name of the light
    std::string m_Name;

    // the byte values of the light
    uint8_t *m_bytes;
    unsigned int m_size;

    // the description of the light patches
    std::vector<LightPatchUnit> m_patchID;

    // map patch unit to indexes
    std::unordered_map<LightPatchUnit, std::vector<unsigned int>> m_patchMap;

    // map group name to group
    std::unordered_map<std::string, std::vector<Group>> m_groupMap;

    unsigned int start = 0;

    Light(std::string name, std::vector<LightPatchUnit> patchUnits) : m_Name(name), m_patchID(patchUnits)
    {
        unsigned int counter = 0;
        for (unsigned int i = 0; i < patchUnits.size(); i++)
        {
            LightPatchUnit unit = patchUnits[i];
            unsigned int size = LIGHT_PATCH_UNIT_SIZE(unit);

            m_patchMap[unit].push_back(counter);
            counter += size;
        }

        m_size = counter;
    }

    Light(std::string name, Group group, unsigned int numGroups) : m_Name(name)
    {
        m_size = group.getGroupFootprint() * numGroups;
        for (int i = 0; i < numGroups; i++)
        {
            m_patchID.insert(m_patchID.end(), group.units.begin(), group.units.end());
        }

        unsigned int counter = 0;
        for (int i = 0; i < numGroups; i++)
        {
            for (int j = 0; j < group.getNumUnits(); j++)
            {
                LightPatchUnit unit = group.units[j];
                unsigned int size = LIGHT_PATCH_UNIT_SIZE(unit);
                m_patchMap[unit].push_back(counter);
                counter += size;
            }
        }

        addGroup(group);
    }

    inline const unsigned int getSize() const { return m_size; }

    inline uint8_t *getBytes() const { return m_bytes; }

    // sets all unit patches of the light to a value
    void set(LightPatchUnit unit, int value)
    {
        auto &indices = m_patchMap[unit];
        unsigned int size = LIGHT_PATCH_UNIT_SIZE(unit);
        for (auto &index : indices)
        {
            for (int i = 0; i < size; i++)
            {
                m_bytes[index + size - i - 1] = (value >> (i * 8)) & 0xFF;
            }
        }
    }

    // adds a custom indexing group
    bool addGroup(Group group)
    {
        if (m_groupMap.find(group.name) != m_groupMap.end())
        {
            throw std::runtime_error("Group already exists");
            return false;
        }

        // check if sequence exists
        int offset = 0;
        for (int i = 0; i < m_patchID.size(); i++)
        {
            bool found = true;
            for (int j = 0; j < group.units.size(); j++)
            {
                if (m_patchID[i + j] != group.units[j])
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                Group newGroup = group;
                newGroup.memory = &m_bytes;
                newGroup.offset = offset;
                m_groupMap[group.name].push_back(newGroup);
            }
            offset += LIGHT_PATCH_UNIT_SIZE(m_patchID[i]);
        }

        return true;
    }

    // sets group values at index
    bool setGroup(std::string name, unsigned int index, std::vector<int> values)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        auto &group = m_groupMap[name][index];
        return group.setValues(values);
    }

    // sets group values at indexes
    bool setGroup(std::string name, std::vector<unsigned int> indexes, std::vector<int> values)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        for (auto index : indexes)
        {
            if (!setGroup(name, index, values))
                return false;
        }

        return true;
    }

    // sets all groups to values
    bool setAllGroups(std::string name, std::vector<std::vector<int>> values)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        for (int i = 0; i < m_groupMap[name].size(); i++)
        {
            if (!setGroup(name, i, values[i]))
                return false;
        }

        return true;
    }

    // get reference to a group at index
    Group &getGroup(std::string name, unsigned int index)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
        }

        return m_groupMap[name][index];
    }

    // get all groups at name
    std::vector<Group> &operator[](std::string name)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
        }

        return m_groupMap[name];
    }

    // return values of all groups
    std::vector<std::vector<int>> getGroups(std::string name)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
        }

        std::vector<std::vector<int>> values;
        for (int i = 0; i < m_groupMap[name].size(); i++)
        {
            values.push_back(m_groupMap[name][i].getValues());
        }

        return values;
    }

    // apply a lambda function to a group at index
    bool applyFunctionToGroup(std::string name, int index, EffectParams params, EffectFn func)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        params.localId = index;
        auto values = getGroups(name);
        auto &setVals = values[params.localId];
        params.localSize = m_groupMap[name].size();
        if (params.globalSize < 0) params.globalSize = params.localSize;
        func(params, values);
        return setGroup(name, params.localId, setVals);
    }

    // apply a lambda function to a group at indexes
    bool applyFunctionToGroup(std::string name, std::vector<unsigned int> indexes, EffectParams params, EffectFn func)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        auto values = getGroups(name);
        params.localSize = m_groupMap[name].size();
        if (params.globalSize < 0) params.globalSize = params.localSize;
        for (auto index : indexes)
        {
            params.localId = index;
            auto &setVals = values[params.localId];
            func(params, values);
            if(!setGroup(name, params.localId, setVals))
                return false;
        }

        return true;
    }

    // apply a lambda function to all groups
    bool applyFunctionToAllGroups(std::string name, EffectParams params, EffectFn func)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        auto values = getGroups(name);
        params.localSize = m_groupMap[name].size();
        if (params.globalSize < 0) params.globalSize = params.localSize;
        for (int i = 0; i < m_groupMap[name].size(); i++)
        {
            params.localId = i;
            auto &setVals = values[params.localId];
            func(params, values);
            if(!setGroup(name, params.localId, setVals))
                return false;
        }

        return true;
    }

    std::string toString() const
    {
        std::stringstream ss;
        ss << colorItalic << m_Name << colorReset << std::endl;
        ss << "Groups:\n";
        for (const auto &[groupName, groups] : m_groupMap)
        {
            ss << " - " << groups[0].describe() << std::endl;
        }
        return ss.str();
    }

    // // PRINTING FUNCTIONS
    void printGroups() const
    {
        for (const auto &[groupName, groups] : m_groupMap)
        {
            std::cout << " - " << groups[0].describe() << std::endl;
        }
    }

    void printBytesPatched() const
    {
        std::cout << colorDim;
        for (int i = 0; i < m_size; i++)
        {
            if (i % 16 == 0)
                std::cout << std::endl;

            std::cout << Utils::padByte(m_bytes[i], 3) << " ";
        }

        std::cout << colorReset << std::endl;
    }

    void print() const
    {
        std::cout << colorItalic << "[" << m_Name << "]" << colorReset << std::endl;
        std::cout << "Groups:\n";
        printGroups();
        std::cout << "Bytes:";
        printBytesPatched();
    }
};

bool applyFunctionToLights(std::vector<Light> lights, std::string group, EffectParams params, EffectFn fun)
{
    params.globalSize = 0;
    params.offsetGlobal = 0;
    for (auto& light : lights)
    {
        params.globalSize += light[group].size();
    }
    for (auto& light : lights)
    {
        if(!light.applyFunctionToAllGroups(group, params, fun))
            return false;
        params.offsetGlobal += light[group].size();
    }
    return true;
}

struct DMXUniverse
{
    unsigned int universeID;

    std::vector<Light> lights;
    std::unordered_map<std::string, std::vector<Light>> ligthsByName;
    uint8_t m_bytes[MAX_SIZE] = {0};
    unsigned int bytesPatched[MAX_SIZE] = {0};

    DMXUniverse(unsigned int universe = 1) : universeID(universe) {}

    void fillBytesPatched(int start, int end)
    {
        static unsigned int num = 1;
        for (int i = start; i < end; i++)
        {
            bytesPatched[i] = num;
        }

        num++;
    }

    bool add(Light &fragment, int start = -1)
    {
        if (lights.empty())
        {
            int size = fragment.getSize();
            if (size <= MAX_SIZE)
            {
                unsigned int startOffset = start == -1 ? 0 : start;
                fragment.start = startOffset;
                fragment.m_bytes = &m_bytes[startOffset];
                lights.push_back(fragment);

                fillBytesPatched(startOffset, startOffset + size);
                return true;
            }
            else
            {
                std::cout << "Segment exceeds max size" << std::endl;
                return false;
            }
        }

        // insert next
        if (start == -1)
        {
            auto &frag = lights[lights.size() - 1];

            // if segment fits at end
            if (frag.start + frag.getSize() + fragment.getSize() <= MAX_SIZE)
            {
                fragment.start = frag.start + frag.getSize();
                fragment.m_bytes = &m_bytes[fragment.start];
                lights.push_back(fragment);
                fillBytesPatched(fragment.start, fragment.start + fragment.getSize());
                return true;
            }
        }
        else
        {
            unsigned int size = fragment.getSize();
            for (int i = 0; i < lights.size(); i++)
            {
                auto &current = lights[i];

                // not last segment
                if (i < lights.size() - 1)
                {
                    auto &next = lights[i + 1];
                    if (start >= current.start && start < current.start + current.getSize())
                    {
                        std::cout << "Segment within bounds" << std::endl;
                        return false;
                    }
                    // same for next
                    if (start >= next.start && start < next.start + next.getSize())
                    {
                        std::cout << "Segment within bounds" << std::endl;
                        return false;
                    }
                    if (current.start + current.getSize() <= start && start + size <= next.start)
                    {
                        fragment.start = start;
                        fragment.m_bytes = &m_bytes[fragment.start];
                        lights.insert(lights.begin() + i + 1, fragment);
                        fillBytesPatched(start, start + size);
                        return true;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    // check if segment within current segment bounds
                    if (start >= current.start && start < current.start + current.getSize())
                    {
                        std::cout << "Segment within bounds 1" << std::endl;
                        return false;
                    }

                    if (start >= MAX_SIZE)
                    {
                        std::cout << "Segment start is out of bounds 2" << std::endl;
                        return false;
                    }

                    if (start + size <= MAX_SIZE)
                    {
                        fragment.start = start;
                        fragment.m_bytes = &m_bytes[fragment.start];
                        lights.push_back(fragment);
                        fillBytesPatched(start, start + size);
                        return true;
                    }
                    else
                    {
                        std::cout << "Segment exceeds max size" << std::endl;
                        return false;
                    }
                }
            }
        }

        return false;
    }

    inline const unsigned int numLights() const { return lights.size(); }

    Light &operator[](int index)
    {
        if (index < 0 || index >= lights.size())
        {
            throw std::runtime_error("Index out of bounds");
        }
        return lights[index];
    }

    std::vector<uint8_t> getBytes() const
    {
        std::vector<uint8_t> bytes(MAX_SIZE, 0);
        int index = 0;
        for (int i = 0; i < lights.size(); i++)
        {
            const auto &curr = lights[i];
            uint8_t *currBytes = curr.getBytes();

            for (int j = curr.start; j < curr.start + curr.getSize(); j++)
            {
                bytes[j] = currBytes[j - curr.start];
            }
        }

        return bytes;
    }

    void printFragments() const
    {
        // std::cout << "Segments: " << segments.size() << std::endl;
        for (int i = 0; i < lights.size(); i++)
        {
            const int offset = 0;
            auto &curr = lights[i];
            int cstart = curr.start;
            int cnext = curr.start + curr.getSize();
            int cend = curr.start + curr.getSize() - 1;

            if (cstart != 0 && i == 0)
            {
                printf("[%3d, %3d] Empty\n", 0 + offset, cstart - 1 + offset);
                // std::cout << "[" << std::setw(3) << 0 + offset << ", " << std::setw(3) << (cstart - 1 + offset) << "]"
                //           << " Empty" << std::endl;
            }

            printf("[%3d, %3d] %s\n", cstart + offset, cend + offset, curr.m_Name.c_str());
            // printf("[%3d, %3d] %s\n", cstart + offset, cend + offset, curr.toString().c_str());
            // std::cout << "[" << std::setw(3) << (curr.start + offset) << ", " << std::setw(3) << (curr.start + curr.getSize() - 1 + offset) << "] " << curr.toString() << std::endl;

            if (i < lights.size() - 1)
            {
                auto &next = lights[i + 1];
                if (curr.start + curr.getSize() != next.start)
                {
                    printf("[%3d, %3d] Empty\n", cnext + offset, next.start - 1 + offset);
                    // std::cout << "[" << std::setw(3) << (curr.start + curr.getSize() + offset) << ", " << std::setw(3) << (next.start - 1 + offset) << "]"
                    //           << " Empty" << std::endl;
                    continue;
                }
            }
            else
            {
                if (curr.start + curr.getSize() < MAX_SIZE)
                {
                    printf("[%3d, %3d] Empty\n", cnext + offset, MAX_SIZE - 1 + offset);
                    // std::cout << "[" << std::setw(3) << (curr.start + curr.getSize() + offset) << ", " << std::setw(3) << (MAX_SIZE - 1 + offset) << "]"
                    //           << " Empty" << std::endl;
                    continue;
                }
            }
        }
    }

    void printBytes() const
    {
        unsigned int prevByteColor = 0;
        std::vector<float> colorVecHsv = {110.0f, 0.7f, 1.0f};
        std::vector<uint8_t> bytes = getBytes();

        const std::string reset = "\x1B[0m";
        std::string col = reset;

        std::cout << "\x1B[3m";

        int cond = std::min((int)bytes.size(), 16);
        for (int i = 0; i < cond; i++)
        {
            static const std::string hex = "0123456789ABCDEF";
            std::cout << "0x" << hex[i] << "  ";
        }

        std::cout << std::dec << reset << std::endl;

        // print seperator
        for (int i = 0; i < cond; i++)
        {
            std::cout << "----";
            if (i < cond - 1)
                std::cout << "-";
        }

        for (int i = 0; i < bytes.size(); i++)
        {
            if (i % 16 == 0)
                std::cout << std::endl;

            if (bytesPatched[i] != 0)
            {
                if (prevByteColor != bytesPatched[i])
                {
                    auto colorVec = Utils::hsvToRgb(colorVecHsv);
                    col = Utils::colorByRGB(colorVec[0], colorVec[1], colorVec[2], true);
                    colorVecHsv[0] = fmod(colorVecHsv[0] + 55, 360.0f);

                    prevByteColor = bytesPatched[i];
                }
            }
            else
            {
                col = reset + "\x1B[2m" + "\x1B[3m";
            }
            std::cout << col << Utils::padByte(bytes[i], 3) << reset << "  ";
        }

        std::cout << std::endl;
    }

    void print() const
    {
        std::cout << "Universe: " << universeID << std::endl;
        std::cout << "Number of Lights: " << lights.size() << std::endl;
        std::cout << "Patch list: " << std::endl;
        printFragments();

        std::cout << "Bytes: " << std::endl;
        printBytes();
    };
};

int main()
{

    DMXUniverse memory(1);
    DMXUniverse memory2(2);
    Light::Group rgb("RGB", {R, G, B});
    Light light("LedBar", rgb, 20);
    Light light2("LedBar2", rgb, 30);
    Light light3("LedBar3", rgb, 40);

    if (!memory.add(light))
    {
        return -1;
    }

    if (!memory.add(light2))
    {
        return -1;
    }

    if (!memory2.add(light3))
    {
        return -1;
    }

    // for (int i = 0; i < 10; i++)
    // {
    //     Light temp("LedBar4", rgb, 1);
    //     memory.add(temp);
    // }

    EffectFn redBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        v[0] = (int)(std::sin(M_PI * ((params.localId + params.tick) % params.localSize) / params.localSize) * 255);
        v[2] = (int)(std::sin(M_PI * ((params.localId + params.tick + params.localSize / 2) % params.localSize) / params.localSize) * 255);
    };

    EffectFn lowerIntesityLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        auto hsv = Utils::rgbToHsv(v);
        hsv[2] *= 0.9;
        auto rgb = Utils::hsvToRgb(hsv);
        v[0] = rgb[0];
        v[1] = rgb[1];
        v[2] = rgb[2];
    };

    EffectFn coloriseLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    };

    EffectFn hueShiftLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        auto hsv = Utils::rgbToHsv(v);
        hsv[0] = fmod(hsv[0] + 5 * params.tick, 360.0);
        auto rgb = Utils::hsvToRgb(hsv);
        v[0] = rgb[0];
        v[1] = rgb[1];
        v[2] = rgb[2];
    };

    EffectFn globalSnake = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        float globalId = (float)params.offsetGlobal+params.localId;
        if(params.tick % params.globalSize == globalId)
        {
            auto rgb = Utils::hsvToRgb({(globalId/params.globalSize*359.0f), 1.0, 1.0});
            v[0] = rgb[0];
            v[1] = rgb[1];
            v[2] = rgb[2];
        }
        else
        {
            auto hsv = Utils::rgbToHsv(v);
            hsv[2] *= 0.9;
            auto rgb = Utils::hsvToRgb(hsv);
            v[0] = rgb[0];
            v[1] = rgb[1];
            v[2] = rgb[2];
        }
    };

    EffectParams params(10);
    light2.applyFunctionToAllGroups("RGB", params, redBlueLambda);
    for (int i = 0; i < 100; i++)
    {
        params.tick = i;
        applyFunctionToLights({memory[0], memory[1], memory2[0]}, "RGB", params, globalSnake);
        // memory[0].applyFunctionToAllGroups("RGB", params, redBlueLambda);
        memory[0].applyFunctionToAllGroups("RGB", params, coloriseLambda);

        // light2.applyFunctionToAllGroups("RGB", params, hueShiftLambda);
        light2.applyFunctionToAllGroups("RGB", params, coloriseLambda);

        // light3.setGroup("RGB", i % 40, Utils::hsvToRgb({(float)fmod((float)i * 5, 360.0f), 1.0, 1.0}));
        // light3.applyFunctionToAllGroups("RGB", params, lowerIntesityLambda);
        light3.applyFunctionToAllGroups("RGB", params, coloriseLambda);
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "\r";
    }
    std::cout << std::endl;
    // memory[memory.numLights() - 1].setGroup("RGB", 0, {255, 0, 0});

    memory.print();
}