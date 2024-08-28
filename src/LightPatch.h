#pragma once
#include <vector>
#include <string>

#include "PatchUnit.h"

class LightPatch
{
    unsigned int m_ID = 0;
    std::string m_Name;
    unsigned int m_Size = 0;

    std::vector<PatchUnit> m_patchUnits;

public:
    LightPatch(std::string name = "");
    LightPatch(std::string name, std::initializer_list<PatchUnit> patchUnits);

    inline const unsigned int size() const { return m_Size; }
    PatchUnit &operator[](int index);

    bool addPatchUnit(PatchUnit type);
    std::vector<uint8_t> byteValue() const;

    void printPatchUnits(int offset) const;
    void printLight(int offset) const;
};
