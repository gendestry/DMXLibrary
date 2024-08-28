#include "DMXUniverse.h"
#include "Utils.h"

#include <iostream>

void DMXUniverse::addLightPatch(LightPatch patch)
{
    if (m_Size + patch.size() > 512)
    {
        throw std::runtime_error("Patch size exceeds 512 bytes");
        return;
    }

    m_LightPatches.push_back(patch);
    m_Size += patch.size();
}

std::vector<uint8_t> DMXUniverse::byteValue() const
{
    std::vector<uint8_t> bytes;
    for (const auto &patch : m_LightPatches)
    {
        auto patchBytes = patch.byteValue();
        bytes.insert(bytes.end(), patchBytes.begin(), patchBytes.end());
    }
    return bytes;
}

LightPatch &DMXUniverse::operator[](int index)
{
    return m_LightPatches[index];
};

void DMXUniverse::printLightPatches()
{
    std::cout << "Num lights: " << m_LightPatches.size() << ", Total size: " << (m_Size / 8) << " Channels" << std::endl;
    int prevOffset = 0;
    unsigned int c = 0;
    for (const auto &patch : m_LightPatches)
    {
        std::cout << "[" << c++ << "] ";
        patch.printLight(prevOffset);
        prevOffset += patch.size();
    }
}

void DMXUniverse::printByteValue() const
{
    std::vector<uint8_t> bytes = byteValue();
    std::cout << "Bytes: " << "\x1B[32m";
    int i = 0;
    for (; i < bytes.size(); i++)
    {
        if (i % 16 == 0)
            std::cout << std::endl;
        std::cout << padByte(bytes[i], 3) << "  ";
    }

    std::cout << "\x1B[0m";
    while (i < 512)
    {
        if (i % 16 == 0)
            std::cout << std::endl;
        std::cout << padByte(0, 3) << "  ";

        i++;
    }
    std::cout << std::endl;
}
