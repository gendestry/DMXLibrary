#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include "LightGroup.h"

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

        Effect(EffectType type, EffectFn fun) : m_Type(type), m_Fun(fun) {}

        volatile int getType() const { return (int)m_Type; }
        volatile void apply(LightGroup &group, std::string groupName, EffectParams &params)
        {
            applyFunctionToLights(group, groupName, params, m_Fun);
        }
    };

    struct IntensityEffect : public Effect
    {
        IntensityEffect(EffectFn fun) : Effect(Intensity, fun) {}
    };

    struct ColorEffect : public Effect
    {
        ColorEffect(EffectFn fun) : Effect(Color, fun) {}
    };

    struct OtherEffect : public Effect
    {
        OtherEffect(EffectFn fun) : Effect(Other, fun) {}
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