#include "Light.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace DMX
{
    std::unordered_map<std::string, unsigned int> Light::m_currentIndex;

    Light::Group::Group(std::string name, std::vector<LightPatchUnit> units) : name(name), units(units)
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

    std::vector<int> Light::Group::getValues() const
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

    bool Light::Group::setValues(std::vector<int> values)
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

    std::string Light::Group::describe() const
    {
        std::stringstream ss;
        // printf("Ptr: %p\n", memory);
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

    std::string Light::Group::toString() const
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

    /* ===== LIGHT CLASS ===== */
    Light::Light(std::string name, std::vector<LightPatchUnit> patchUnits, bool standalone) : m_Name(name), m_patchID(patchUnits), m_standalone(standalone)
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

        if (standalone)
            m_bytes = new uint8_t[m_size];
    }

    Light::Light(std::string name, Group group, unsigned int numGroups, bool standalone) : m_Name(name), m_standalone(standalone)
    {
        m_size = group.getGroupFootprint() * numGroups;

        if (standalone)
            m_bytes = new uint8_t[m_size];

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

    Light::~Light()
    {
        if (m_standalone)
            delete[] m_bytes;
    }

    // copy constructor
    Light::Light(const Light &other)
        : m_Name(other.m_Name),
          start(other.start),
          m_size(other.m_size),
          m_patchID(other.m_patchID),
          m_patchMap(other.m_patchMap),
          m_groupMap(other.m_groupMap),
          m_bytes(other.m_bytes),
          m_ID(other.m_ID)
    {
        refreshGroups();
    }

    unsigned int Light::incrementID(std::string name)
    {
        if (m_currentIndex.find(name) == m_currentIndex.end())
        {
            m_currentIndex[name] = 0;
        }
        return ++m_currentIndex[name];
    }

    // sets all unit patches of the light to a value
    void Light::set(LightPatchUnit unit, int value)
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
    bool Light::addGroup(Group group)
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

    // used to refresh groups' memory pointers
    void Light::refreshGroups()
    {
        for (auto &[name, groups] : m_groupMap)
        {
            auto group = groups[0];
            groups.clear();
            m_groupMap.erase(name);
            addGroup(group);
        }
    }

    // sets group values at index
    bool Light::setGroup(std::string name, unsigned int index, std::vector<int> values)
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
    bool Light::setGroup(std::string name, std::vector<unsigned int> indexes, std::vector<int> values)
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
    bool Light::setAllGroups(std::string name, std::vector<std::vector<int>> values)
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
    Light::Group &Light::getGroup(std::string name, unsigned int index)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
        }

        return m_groupMap[name][index];
    }

    // get all groups at name
    std::vector<Light::Group> &Light::operator[](std::string name)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
        }

        return m_groupMap[name];
    }

    // return values of all groups
    std::vector<std::vector<int>> Light::getGroups(std::string name)
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
    bool Light::applyFunctionToGroup(std::string name, int index, Effect::EffectParams params, Effect::EffectFn func)
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
        if (params.globalSize < 0)
            params.globalSize = params.localSize;
        func(params, values);
        return setGroup(name, params.localId, setVals);
    }

    // apply a lambda function to a group at indexes
    bool Light::applyFunctionToGroup(std::string name, std::vector<unsigned int> indexes, Effect::EffectParams params, Effect::EffectFn func)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        auto values = getGroups(name);
        params.localSize = m_groupMap[name].size();
        if (params.globalSize < 0)
            params.globalSize = params.localSize;
        for (auto index : indexes)
        {
            params.localId = index;
            auto &setVals = values[params.localId];
            func(params, values);
            if (!setGroup(name, params.localId, setVals))
                return false;
        }

        return true;
    }

    // apply a lambda function to all groups
    bool Light::applyFunctionToAllGroups(std::string name, Effect::EffectParams params, Effect::EffectFn func)
    {
        if (m_groupMap.find(name) == m_groupMap.end())
        {
            throw std::runtime_error("Group not found");
            return false;
        }

        auto values = getGroups(name);
        params.localSize = m_groupMap[name].size();
        if (params.globalSize < 0)
            params.globalSize = params.localSize;
        for (int i = 0; i < m_groupMap[name].size(); i++)
        {
            params.localId = i;
            auto &setVals = values[params.localId];
            func(params, values);
            if (!setGroup(name, params.localId, setVals))
                return false;
        }

        return true;
    }

    // // PRINTING FUNCTIONS
    std::string Light::toString() const noexcept
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

    void Light::printGroups() const noexcept
    {
        for (const auto &[groupName, groups] : m_groupMap)
        {
            std::cout << " - " << groups[0].describe() << std::endl;
        }
    }

    void Light::printBytesPatched() const noexcept
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

    void Light::print() const noexcept
    {
        std::cout << colorGreen << m_Name << " [" << m_ID << "]" << colorReset << std::endl;
        std::cout << "Groups:\n";
        printGroups();
        std::cout << "Bytes:";
        printBytesPatched();
    }

};
