#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <cmath>

const std::string colorGreen = "\x1B[32m";
const std::string colorYellow = "\x1B[33m";
const std::string colorBlue = "\x1B[34m";
const std::string colorReset = "\x1B[0m";
const std::string colorDim = "\x1B[2m";
const std::string colorItalic = "\x1B[3m";

std::vector<uint8_t> toBytes(int numBytes, int number)
{
    std::vector<uint8_t> bytes(numBytes);
    for (int i = 0; i < numBytes; i++)
    {
        bytes[numBytes - i - 1] = (number >> (i * 8)) & 0xFF;
    }
    return bytes;
}

int toNumber(std::vector<uint8_t> bytes)
{
    int number = 0;
    int size = bytes.size();
    for (int i = 0; i < bytes.size(); i++)
    {
        number |= bytes[size - i - 1] << (i * 8);
    }
    return number;
}

std::string padByte(int num, int pad)
{
    std::string str = std::to_string(num);
    int len = str.length();
    if (len < pad)
    {
        str.insert(0, pad - len, ' ');
    }
    return str;
}

std::string colorByRGB(int r, int g, int b, bool fg)
{
    std::stringstream ss;
    ss << "\x1B[" << (fg ? 3 : 4) << "8;2;" << r << ";" << g << ";" << b << "m";
    return ss.str();
}

std::vector<float> linearInterpolation(float start, float end, int n)
{
    std::vector<float> values(n);

    float step = (end - start) / (n - 1);
    for (int i = 0; i < n; i++)
    {
        values[i] = start + i * step;
    }

    return values;
}

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

struct Light
{
    struct Group
    {
        std::string name;
        std::vector<LightPatchUnit> units;
        std::vector<unsigned int> offsets;
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

        unsigned int getGroupSize() const
        {
            return size;
        }

        unsigned int getNumUnits() const
        {
            return units.size();
        }

        std::string toString() const
        {
            std::stringstream ss;
            std::cout << "\x1B[34m\"" << name << "\"\x1B[0m size: " << size << " {";
            for (int i = 0; i < units.size(); i++)
            {
                if (i != 0)
                    ss << ", ";
                ss << LIGHT_PATCH_UNIT_TO_STRING(units[i]) << ": " << colorDim << offsets[i] << colorReset;
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

    std::string m_Name;

    // the byte values of the light
    std::vector<uint8_t> m_bytes;

    // the description of the light patches
    std::vector<LightPatchUnit> m_patchID;

    // map patch unit to indexes
    std::unordered_map<LightPatchUnit, std::vector<unsigned int>> m_patchMap;

    // map group name to group
    std::unordered_map<std::string, Group> m_groupMap;

    // map group name to group indexes
    std::unordered_map<std::string, std::vector<unsigned int>> m_groupIndexMap;

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

        m_bytes.resize(counter);
    }

    Light(std::string name, Group group, unsigned int numGroups) : m_Name(name)
    {
        m_bytes.resize(group.getGroupSize() * numGroups);
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
            return false;
        }

        // check if sequence exists
        m_groupMap.insert({group.name, group});

        int offset = 0;
        for (int i = 0; i < m_patchID.size(); i++)
        {
            // LightPatchUnit unit = m_patchID[i];
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
                m_groupIndexMap[group.name].push_back(offset);
            }
            offset += LIGHT_PATCH_UNIT_SIZE(m_patchID[i]);
        }

        return true;
    }

    // sets group values at index
    bool setGroup(std::string name, unsigned int index, std::vector<int> values)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return false;
        }
        auto &group = m_groupMap[name];
        auto &index2 = m_groupIndexMap[name][index];

        for (int i = 0; i < group.offsets.size(); i++)
        {
            if (values[i] == -1)
                continue;
            auto &unit = group.units[i];
            unsigned int size = LIGHT_PATCH_UNIT_SIZE(unit);
            for (int j = 0; j < size; j++)
            {
                m_bytes[index2 + group.offsets[i] + size - j - 1] = (values[i] >> (j * 8)) & 0xFF;
            }
        }
        return true;
    }

    // sets group values at indexes
    bool setGroup(std::string name, std::vector<unsigned int> indexes, std::vector<int> values)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return false;
        }

        for (auto index : indexes)
        {
            if (!setGroup(name, index, values))
                return false;
        }

        return true;
    }

    bool setAllGroups(std::string name, std::vector<std::vector<int>> values)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return false;
        }

        for (int i = 0; i < m_groupIndexMap[name].size(); i++)
        {
            if (!setGroup(name, i, values[i]))
                return false;
        }

        return true;
    }

    // returns int values of group at index
    std::vector<int> getGroup(std::string name, unsigned int index)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return std::vector<int>();
        }

        auto &group = m_groupMap[name];
        auto &index2 = m_groupIndexMap[name][index];

        std::vector<int> values(group.getNumUnits());
        for (int i = 0; i < group.getNumUnits(); i++)
        {
            auto &unit = group.units[i];
            unsigned int size = LIGHT_PATCH_UNIT_SIZE(unit);
            int value = 0;
            for (int j = 0; j < size; j++)
            {
                value |= m_bytes[index2 + group.offsets[i] + size - j - 1] << (j * 8);
            }
            values[i] = value;
        }
        return values;
    }

    // returns int values of group at indexes
    std::vector<std::vector<int>> getGroups(std::string name)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return std::vector<std::vector<int>>();
        }

        auto &group = m_groupMap[name];
        std::vector<std::vector<int>> values;
        for (int i = 0; i < m_groupIndexMap[name].size(); i++)
        {
            values.push_back(getGroup(name, i));
        }
        return values;
    }

    // apply a lambda function to a group at index
    bool applyFunctionToGroup(std::string name, unsigned int index, void (*func)(int, int, int, std::vector<std::vector<int>> &), int offset = 0)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return false;
        }

        auto values = getGroups(name);
        func(m_groupIndexMap[name].size(), index, offset, values);
        setAllGroups(name, values);

        return true;
    }

    // apply a lambda function to a group at indexes
    bool applyFunctionToGroup(std::string name, std::vector<unsigned int> indexes, void (*func)(int, int, int, std::vector<std::vector<int>> &), int offset = 0)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return false;
        }

        for (auto index : indexes)
        {
            auto values = getGroups(name);
            func(m_groupIndexMap[name].size(), index, offset, values);
            if (!setAllGroups(name, values))
                return false;
        }

        return true;
    }

    // apply a lambda function to all groups
    bool applyFunctionToAllGroups(std::string name, void (*func)(int, int, int, std::vector<std::vector<int>> &), int offset = 0)
    {
        if (m_groupIndexMap.find(name) == m_groupIndexMap.end())
        {
            return false;
        }

        for (int i = 0; i < m_groupIndexMap[name].size(); i++)
        {
            auto values = getGroups(name);
            func(m_groupIndexMap[name].size(), i, offset, values);
            if (!setAllGroups(name, values))
                return false;
        }

        return true;
    }

    // PRINTING FUNCTIONS
    void printGroups()
    {
        for (auto &[groupName, group] : m_groupMap)
        {
            // print groupindexsize
            std::cout << " - [" << m_groupIndexMap[groupName].size() << "] " << group.toString() << std::endl;
        }
    }

    void printBytesPatched() const
    {
        std::cout << colorDim;
        for (int i = 0; i < m_bytes.size(); i++)
        {
            if (i % 16 == 0)
                std::cout << std::endl;

            std::cout << padByte(m_bytes[i], 3) << " ";
        }

        std::cout << colorReset << std::endl;
    }

    void print()
    {
        std::cout << colorItalic << "[" << m_Name << "]" << colorReset << std::endl;
        std::cout << "Groups:\n";
        printGroups();
        std::cout << "Bytes:";
        printBytesPatched();
    }
};

int main()
{
    constexpr unsigned int NLEDS = 30;

    Light::Group group("RGB", {R, G, B});
    Light light2("Bar", group, NLEDS);

    auto sinLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        v[0] = (int)(std::sin(M_PI * ((index + offset) % groupSize) / groupSize) * 255);
    };

    auto redBlueLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        v[0] = (int)(std::sin(M_PI * ((index + offset) % groupSize) / groupSize) * 255);
        v[2] = (int)(std::sin(M_PI * ((index + offset + groupSize / 2) % groupSize) / groupSize) * 255);
    };

    auto toggleRedLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        v[0] = v[0] == 0 ? 255 : 0;
    };

    auto coloriseLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        std::cout << colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    };

    auto globalIntensityLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        for (int i = 0; i < v.size(); i++)
        {
            v[i] = std::max(20, v[i]);
        }
    };

    for (int i = 0; i < NLEDS; i++)
    {
        light2.applyFunctionToAllGroups("RGB", redBlueLambda, i);
        // light2.applyFunctionToAllGroups("RGB", redBlueLambda, i);
        light2.applyFunctionToAllGroups("RGB", coloriseLambda);
        std::cout << std::endl;
    }

    light2.print();

    // for (int i = 0; i < NLEDS; i++)
    // {
    //     light2.setGroup("RGB", i, {i % 3 > 0 ? 255 : 0, 0, 0});
    // }

    // light2.applyFunctionToAllGroups("RGB", coloriseLambda);
    // light2.printBytesPatched();
    // std::cout << std::endl;

    // // light2.applyFunctionToAllGroups("RGB", toggleRedLambda);
    // // light2.applyFunctionToAllGroups("RGB", globalIntensityLambda);
    // light2.applyFunctionToAllGroups("RGB", coloriseLambda);
    // light2.printBytesPatched();

    // light2.print();
    // int n = 5;
    // for (int i = 0; i < n; i++)
    // {
    //     std::cout << std::sin(M_PI * i / n) * 255 << ", ";
    // }
    // light2.addGroup(group);
    // // light2.setGroup("RGB", 0, {0, 255, 0});
    // // light2.setGroup("RGB", 1, {0, 255, 255});
    // // light2.setGroup("RGB", 2, {0, 255, 0});
    // // light2.setGroup("RGB", {1, 2}, {255, -1, -1});
    // light2.print();

    // auto lambda = [](std::vector<int> &values)
    // {
    //     values[1]++;
    //     values[1]++;
    // };

    // light2.applyFunctionToGroups("RGB", {0, 1}, lambda);
    // light2.print();

    // auto groups = light2.getGroups("RGB");
    // for (int i = 0; i < groups.size(); i++)
    // {
    //     for (auto &value : groups[i])
    //     {
    //         std::cout << value << " ";
    //     }
    //     std::cout << std::endl;
    // }

    return 0;
}
