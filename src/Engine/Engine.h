#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "../DMX/Universe.h"
#include "../Effects/LightGroup.h"
#include "../Effects/Effect.h"

namespace Engine
{

    // enum Indexer
    // {
    //     Universe,
    //     Group,
    //     Effect
    // };

    // struct IndexerStruct
    // {
    //     Indexer type;
    //     std::string name;
    //     // unsigned int id;
    // }
    class Engine
    {
        std::unordered_map<unsigned int, DMX::Universe> m_Universes;
        std::unordered_map<std::string, Effect::LightGroup> m_Groups;
        std::unordered_map<std::string, Effect::FX> m_Effects;

    public:
        // Engine();
        // ~Engine();

        DMX::Universe &getUniverse(unsigned int id)
        {
            if (m_Universes.find(id) == m_Universes.end())
            {
                m_Universes[id] = DMX::Universe(id);
            }

            return m_Universes[id];
        }

        Effect::LightGroup &getLightGroup(std::string name)
        {
            return m_Groups[name];
        }

        Effect::FX &getFX(std::string name)
        {
            return m_Effects[name];
        }

        DMX::Universe &operator[](unsigned int id)
        {
            return getUniverse(id);
        }

        Effect::LightGroup &operator[](std::string name)
        {
            return getLightGroup(name);
        }

        // void run();
    };
} // namespace Engine