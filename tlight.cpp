#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <thread>
#include <chrono>
#include "Utils.h"

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

struct Light
{
    struct Group
    {
        std::string name;
        std::vector<LightPatchUnit> units;
        std::vector<unsigned int> offsets;
        uint8_t *memory;
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
                    value |= memory[offsets[i] + size - j - 1] << (j * 8);
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
                    memory[offsets[i] + size - j - 1] = (values[i] >> (j * 8)) & 0xFF;
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
    std::vector<uint8_t> m_bytes;

    // the description of the light patches
    std::vector<LightPatchUnit> m_patchID;

    // map patch unit to indexes
    std::unordered_map<LightPatchUnit, std::vector<unsigned int>> m_patchMap;

    // map group name to group
    std::unordered_map<std::string, std::vector<Group>> m_groupMap;

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
        m_bytes.resize(group.getGroupFootprint() * numGroups);
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
                newGroup.memory = &m_bytes[offset];
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
    bool applyFunctionToGroup(std::string name, unsigned int index, void (*func)(int, int, int, std::vector<std::vector<int>> &), int offset = 0)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        auto values = getGroups(name);
        auto &setVals = values[index];
        func(m_groupMap[name].size(), index, offset, values);
        return setGroup(name, index, setVals);
    }

    // apply a lambda function to a group at indexes
    bool applyFunctionToGroup(std::string name, std::vector<unsigned int> indexes, void (*func)(int, int, int, std::vector<std::vector<int>> &), int offset = 0)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        for (auto index : indexes)
        {
            if (!applyFunctionToGroup(name, index, func, offset))
                return false;
        }

        return true;
    }

    // apply a lambda function to all groups
    bool applyFunctionToAllGroups(std::string name, void (*func)(int, int, int, std::vector<std::vector<int>> &), int offset = 0)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        for (int i = 0; i < m_groupMap[name].size(); i++)
        {
            if (!applyFunctionToGroup(name, i, func, offset))
                return false;
        }

        return true;
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
        for (int i = 0; i < m_bytes.size(); i++)
        {
            if (i % 16 == 0)
                std::cout << std::endl;

            std::cout << padByte(m_bytes[i], 3) << " ";
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

int main()
{
    constexpr unsigned int NLEDS = 30;

    Light::Group group("RGB", {R, G, B});
    Light light("Led Bar", group, NLEDS);

    auto redBlueLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        v[0] = (int)(std::sin(M_PI * ((index + offset) % groupSize) / groupSize) * 255);
        v[2] = (int)(std::sin(M_PI * ((index + offset + groupSize / 2) % groupSize) / groupSize) * 255);
    };

    auto coloriseLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        std::cout << colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    };

    auto hueShiftLambda = [](int groupSize, int index, int offset, std::vector<std::vector<int>> &values)
    {
        auto &v = values[index];
        auto hsv = rgbToHsv(v);
        hsv[0] = fmod(hsv[0] + 5 * offset, 360.0);
        auto rgb = hsvToRgb(hsv);
        v[0] = rgb[0];
        v[1] = rgb[1];
        v[2] = rgb[2];
    };

    light.applyFunctionToAllGroups("RGB", redBlueLambda);

    for (int i = 0; i < NLEDS; i++)
    {
        // light.applyFunctionToAllGroups("RGB", redBlueLambda, i);
        light.applyFunctionToAllGroups("RGB", hueShiftLambda, i);
        light.applyFunctionToAllGroups("RGB", coloriseLambda);
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "\r";
    }
    std::cout << std::endl;
    light.print();

    // light.applyFunctionToAllGroups("RGB", hueShiftLambda, NLEDS / 3 * 2);
    // light["RGB"][0].setValues({255, 0, -1});
    // light.applyFunctionToAllGroups("RGB", coloriseLambda);

    // std::cout << std::endl;
    // std::cout << light["RGB"][0].toString() << std::endl;
    // light.print();

    return 0;
}
