#include "LightPatch.h"

#include <iostream>

LightPatch::LightPatch(std::string name) : m_Name(name)
{
    static unsigned int s_ID = 0;
    if (name.empty())
    {
        m_Name = "LightPatch" + std::to_string(s_ID);
    }
    m_ID = s_ID++;
}

LightPatch::LightPatch(std::string name, std::initializer_list<PatchUnit> patchUnits) : LightPatch(name)
{
    for (const auto &unit : patchUnits)
    {
        if (!addPatchUnit(unit))
        {
            std::cout << "Failed to add patch unit for light " << m_Name << ": " << unit.patchTypeString() << std::endl;
            return;
        }
    }
}
// constructor with initializer list

PatchUnit &LightPatch::operator[](int index)
{
    return m_patchUnits[index];
};

bool LightPatch::addPatchUnit(PatchUnit type)
{
    unsigned int size = type.size;
    if (m_Size + size > 512)
    {
        throw std::runtime_error("Patch size exceeds 512 bytes");
        return false;
    }

    m_patchUnits.push_back({type});
    m_PatchUnitIndexes[type.type].push_back(m_patchUnits.size() - 1);
    m_Size += size;

    return true;
}

std::vector<uint8_t> LightPatch::byteValue() const
{
    std::vector<uint8_t> bytes;
    for (const auto &unit : m_patchUnits)
    {
        auto unitBytes = unit.byteValue();
        bytes.insert(bytes.end(), unitBytes.begin(), unitBytes.end());
    }
    return bytes;
}

void LightPatch::printPatchUnits(int offset) const
{
    std::cout << "Patch: " << std::endl;
    unsigned int currentAddr = 0;
    for (const auto &unit : m_patchUnits)
    {
        std::cout << " - " << unit.patchTypeString() << "[" << (unit.size / 8) << "B], Address: "
                  << (currentAddr + offset) << "-" << (currentAddr + unit.size - 1 + offset) << ", Value: ";
        unit.printByteValue();

        currentAddr += unit.size;
    }
}

void LightPatch::printIndexes() const
{
    // for loop of desctructured map
    for (auto &[type, indexes] : m_PatchUnitIndexes)
    {
        auto str = PatchUnit::getTypeString(type);
        std::cout << str << ": ";
        for (const auto &index : indexes)
        {
            std::cout << index << " ";
        }
        std::cout << std::endl;
    }
}

void LightPatch::printLight(int offset) const
{
    int s = m_Size / 8;
    std::cout << "\'" << m_Name << "': " << s << ((s == 1) ? " Channel" : " Channels") << " - Addresses [" << offset << ", " << (offset + m_Size - 1) << "]" << std::endl;
    printPatchUnits(offset);
}

std::vector<std::vector<unsigned int>> LightPatch::operator[](PatchUnit::PatchType type) const
{
    return {m_PatchUnitIndexes.at(type)};
}

std::vector<unsigned int *> LightPatch::valuesForType(PatchUnit::PatchType type)
{
    std::vector<unsigned int *> values;
    for (const auto &index : m_PatchUnitIndexes.at(type))
    {
        values.push_back(&m_patchUnits[index].value);
    }
    return values;
}