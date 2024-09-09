#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include "PatchUnit.h"

class LightPatch
{
    unsigned int m_ID = 0;
    std::string m_Name;
    unsigned int m_Size = 0;

    std::vector<PatchUnit> m_patchUnits;
    std::unordered_map<PatchUnit::PatchType, std::vector<unsigned int>> m_PatchUnitIndexes;

public:
    LightPatch(std::string name = "");
    LightPatch(std::string name, std::initializer_list<PatchUnit> patchUnits);

    inline const unsigned int size() const { return m_Size; }
    inline const unsigned int patchSize() const { return m_patchUnits.size(); }
    PatchUnit &operator[](int index);

    bool addPatchUnit(PatchUnit type);
    std::vector<uint8_t> byteValue() const;

    void printPatchUnits(int offset) const;
    void printIndexes() const;
    void printLight(int offset) const;

    std::vector<std::vector<unsigned int>> operator[](PatchUnit::PatchType type) const;
    std::vector<unsigned int *> valuesForType(PatchUnit::PatchType type);
    inline std::string getName() const { return m_Name; }

    friend std::ostream &operator<<(std::ostream &os, const LightPatch &patch)
    {
        // os << patch.m_Name << " ID: " << patch.m_ID << " Size: " << patch.m_Size << " Channels: " << patch.m_patchUnits.size();
        os << patch.m_Name << ": " << patch.m_patchUnits.size() << " channels";
        return os;
    }
};
