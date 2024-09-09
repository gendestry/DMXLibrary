#include "PatchUnit.h"

#include <iostream>
#include <math.h>
#include <algorithm>

PatchUnit::PatchUnit(PatchType type, unsigned int value) : type(type), value(value)
{
    size = patchSize();

    if (value >= pow(2, size))
    {
        throw std::runtime_error("Value exceeds patch size, " + std::to_string(value));
        return;
    }
}

std::vector<uint8_t> PatchUnit::byteValue() const
{
    std::vector<uint8_t> bytes;
    unsigned int s = size / 8;
    for (int i = 0; i < s; i++)
    {
        int val = (value >> (i * 8)) & 0xFF;
        bytes.push_back(val);
    }
    std::reverse(bytes.begin(), bytes.end());
    return bytes;
}

unsigned int PatchUnit::byteSize(unsigned int value) const
{
    int ret = 0;
    for (int i = 8, cond = pow(2, i); cond < value; i += 8, cond = pow(2, i), ret++)
        ;
    return ret;
}

void PatchUnit::printByteValue() const
{
    std::cout << value << "(";
    auto bytes = byteValue();
    std::cout << std::hex << "0x";
    if (byteSize(value) % 2 == 1 || value == 0)
        std::cout << "0";
    std::cout << value << std::dec << ")" << std::endl;
    // for (const auto &byte : byteValue())
    // {
    //     std::cout << byte << " ";
    // }
    // std::cout << ")" << std::dec << std::endl;
    // std::cout << "Value[" << value << "]: [0x" << std::hex << value << std::dec << std::endl;
}
