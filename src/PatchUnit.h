#pragma once
#include <vector>
#include <string>

struct PatchUnit
{
    enum PatchType
    {
        COLOR = 0,
        COLOR_R,
        COLOR_G,
        COLOR_B,
        COLOR_W,
        INTENSITY,
        STROBE,
        PAN,
        TILT,

        PAN16,
        TILT16,
    };

    PatchType type;
    unsigned int size = 0;
    unsigned int value;

    PatchUnit(PatchType type, unsigned int value = 0);

    std::vector<uint8_t> byteValue() const;

    unsigned int byteSize(unsigned int value) const;

    inline const unsigned int patchSize() const
    {
        return (unsigned int)type < 9 ? 8 : 16;
    }

    inline void setValue(unsigned int value)
    {
        this->value = value;
    }

    std::string patchTypeString() const;

    void printByteValue() const;
};
