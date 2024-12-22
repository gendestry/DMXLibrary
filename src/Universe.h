#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "Light.h"

#define MAX_SIZE 512

namespace DMX
{

    bool applyFunctionToLights(std::vector<std::reference_wrapper<Light>> lights, std::string group, EffectParams params, EffectFn fun);

    class Universe
    {
        unsigned int universeID;

        std::vector<Light> lights;
        std::unordered_map<std::string, std::vector<Light>> ligthsByName;
        uint8_t m_bytes[MAX_SIZE] = {0};
        unsigned int bytesPatched[MAX_SIZE] = {0};

        std::string nextColor(int index) const;

    public:
        Universe(unsigned int universe = 1) : universeID(universe) {}

        void fillBytesPatched(int start, int end);
        bool add(Light &fragment, int start = -1);
        inline const unsigned int numLights() const { return lights.size(); }
        inline const unsigned int getUniverseID() const { return universeID; }

        Light &operator[](int index);
        std::vector<uint8_t> getBytes() const;
        void printFragments() const;
        void printBytes() const;

        void print() const;
    };

};
