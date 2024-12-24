#pragma once
#include <vector>
#include "Light.h"

namespace DMX
{

    struct LightGroup
    {
        std::vector<Light *> lights;

        inline void add(Light &light)
        {
            lights.push_back(&light);
        }

        inline void add(Light *light)
        {
            lights.push_back(light);
        }

        inline void add(std::vector<Light *> lights)
        {
            lights.insert(lights.end(), lights.begin(), lights.end());
        }

        inline const std::vector<Light *> &getLights() const
        {
            return lights;
        }

        inline unsigned int size() const
        {
            return lights.size();
        }

        LightGroup &operator=(const std::vector<Light *> &lights)
        {
            this->lights = lights;
            return *this;
        }

        LightGroup &operator+=(Light *light)
        {
            this->lights.push_back(light);
            return *this;
        }

        LightGroup &operator+=(const std::vector<Light *> &lights)
        {
            this->lights.insert(this->lights.end(), lights.begin(), lights.end());
            return *this;
        }

        LightGroup &operator+=(LightGroup &other)
        {
            auto otherLights = other();
            this->lights.insert(this->lights.end(), otherLights.begin(), otherLights.end());
            return *this;
        }

        inline const std::vector<Light *> &operator()()
        {
            return lights;
        }
    };

};