#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "LightGroup.h"

#include "Light.h"

#define MAX_SIZE 512

namespace DMX
{

    bool applyFunctionToLights(std::vector<std::reference_wrapper<Light>> lights, std::string groupName, EffectParams params, EffectFn fun);
    bool applyFunctionToLights(std::vector<Light *> lights, std::string groupName, EffectParams params, EffectFn fun);
    bool applyFunctionToLights(LightGroup &group, std::string groupName, EffectParams params, EffectFn fun);

    class Universe
    {
        unsigned int universeID;

        std::vector<Light> lights;
        std::unordered_map<std::string, std::vector<std::reference_wrapper<Light>>> ligthsByName;
        uint8_t m_bytes[MAX_SIZE] = {0};
        unsigned int bytesPatched[MAX_SIZE] = {0};

        std::string nextColor(int index) const;

    public:
        Universe(unsigned int universe = 1) : universeID(universe) {}

        void fillBytesPatched(int start, int end);
        bool add(Light &fragment, int start = -1);
        inline const unsigned int numLights() const { return lights.size(); }
        inline const unsigned int getUniverseID() const { return universeID; }
        inline Light *getLight(int index) { return &lights[index]; }
        std::vector<Light *> getLights(std::string name);

        Light &operator[](int index);
        inline std::vector<Light *> operator[](std::string name) { return getLights(name); }

        std::vector<uint8_t> getBytes() const;
        void printFragments() const;
        void printBytes() const;
        void print() const;
    };

};
