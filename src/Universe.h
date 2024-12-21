#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "Light.h"

#define MAX_SIZE 512

namespace DMX
{

    bool applyFunctionToLights(std::vector<Light> lights, std::string group, EffectParams params, EffectFn fun);

    struct Universe
    {
        unsigned int universeID;

        std::vector<Light> lights;
        std::unordered_map<std::string, std::vector<Light>> ligthsByName;
        uint8_t m_bytes[MAX_SIZE] = {0};
        unsigned int bytesPatched[MAX_SIZE] = {0};

        Universe(unsigned int universe = 1) : universeID(universe) {}

        void fillBytesPatched(int start, int end);
        bool add(Light &fragment, int start = -1);
        inline const unsigned int numLights() const { return lights.size(); }

        Light &operator[](int index);
        std::vector<uint8_t> getBytes() const;
        void printFragments() const;
        void printBytes() const;

        void print() const;
    };

};
