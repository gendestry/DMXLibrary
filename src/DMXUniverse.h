#pragma once
#include <iostream>
#include <unordered_map>
#include "LightPatch.h"

#define MAX_SIZE 512

class DMXUniverse
{
    struct Seg
    {
        int start;
        int size;
        LightPatch light;

        friend std::ostream &operator<<(std::ostream &os, const Seg &seg)
        {
            // os << "Segment: [" << seg.start << ", " << seg.start + seg.size - 1 << "] Size: " << seg.size;
            os << seg.light;
            return os;
        }
    };

    std::vector<Seg> segments;
    std::unordered_map<std::string, std::vector<LightPatch *>> ligthsByName;
    bool isPatched[MAX_SIZE] = {false};

    void fillBytesPatched(int from, int to, bool value = true);

public:
    std::vector<uint8_t> byteValue() const;

    LightPatch &operator[](int index);

    bool addSegment(LightPatch light);
    bool addSegment(int start, LightPatch light);

    void printSegments();
    void printByteValue() const;

    std::vector<LightPatch *> getLightsByName(std::string name);
    std::vector<LightPatch *> operator[](std::string name);
};

// #pragma once
// #include <vector>

// #include "LightPatch.h"

// class DMXUniverse
// {
//     unsigned int m_Size = 0;
//     std::vector<LightPatch> m_LightPatches;

// public:
//     void addLightPatch(LightPatch patch);

//     std::vector<uint8_t> byteValue() const;

//     LightPatch &operator[](int index);

//     void printLightPatches();
//     void printByteValue() const;
// };
