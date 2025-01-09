#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include "LightGroup.h"
#include "../Utils/Utils.h"

namespace Effect
{

    enum EffectType
    {
        Intensity,
        Color,
        Pan,
        Tilt,
        Other
    };

    struct Effect
    {
        EffectType m_Type;
        EffectFn m_Fun;
        uint64_t oldMicros = Utils::micros();
        int step = 0;
        float bpm;

        bool shouldUpdate()
        {
            float freq = 1000000.f / bpm;
            auto microst = Utils::micros();
            auto dt = microst - oldMicros;
            // std::cout << "BPM: " << bpm;

            if (dt > freq)
            {
                oldMicros = microst;
                // std::cout << "  " << dt << std::endl;
                return true;
            }

            // std::cout << std::endl;

            return false;
        }

        Effect(EffectType type, EffectFn fun, float speedBPM) : m_Type(type), m_Fun(fun), bpm(speedBPM)
        {
        }

        int getType() const { return (int)m_Type; }
        void apply(LightGroup &group, std::string groupName, EffectParams &params)
        {
            if (shouldUpdate())
            {
                step++;
            }
            params.step = step;
            applyFunctionToLights(group, groupName, params, m_Fun);
        }
    };

    struct IntensityEffect : public Effect
    {
        IntensityEffect(EffectFn fun, float speedBPM = 60.f) : Effect(Intensity, fun, speedBPM) {}
    };

    struct ColorEffect : public Effect
    {
        ColorEffect(EffectFn fun, float speedBPM = 60.f) : Effect(Color, fun, speedBPM) {}
    };

    struct OtherEffect : public Effect
    {
        OtherEffect(EffectFn fun, float speedBPM = 60.f) : Effect(Other, fun, speedBPM) {}
    };

    struct FX
    {
        // DMX::LightGroup group;
        std::unordered_map<EffectType, std::vector<Effect *>> map;
        // std::vector<Effect*> effects;

        // FX(LightGroup &grp) : group(grp) {}

        template <EffectType eType>
        void add(Effect *effect)
        {
            map[eType].push_back(effect);
        }

        template <EffectType eType>
        std::vector<Effect *> &get()
        {
            return map[eType];
        }

        void apply(LightGroup &group, std::string name, EffectParams &params)
        {
            std::vector<Effect *> effects = get<Intensity>();
            // std::cout << "Applying intensity effects " << effects.size() << std::endl;
            for (Effect *effect : effects)
            {
                effect->apply(group, name, params);
            }

            effects = get<Color>();

            // std::cout << "Applying color effects " << effects.size() << std::endl;
            for (Effect *effect : effects)
            {
                effect->apply(group, name, params);
            }

            effects = get<Other>();

            // std::cout << "Applying other effects " << effects.size() << std::endl;
            for (Effect *effect : effects)
            {
                effect->apply(group, name, params);
            }
        }
    };

    // struct IntensityEffect : public Effect {};
};