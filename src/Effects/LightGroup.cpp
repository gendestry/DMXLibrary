#include "LightGroup.h"

using DMX::Light;

namespace Effect
{

    bool applyFunctionToLights(std::vector<std::reference_wrapper<Light>> lights, std::string groupName, EffectParams params, EffectFn fun)
    {
        params.globalSize = 0;
        params.offsetGlobal = 0;
        for (auto &light : lights)
        {
            params.globalSize += light.get()[groupName].size();
        }

        for (auto &light : lights)
        {
            if (!light.get().applyFunctionToAllGroups(groupName, params, fun))
                return false;
            params.offsetGlobal += light.get()[groupName].size();
        }
        return true;
    }

    bool applyFunctionToLights(std::vector<Light *> lights, std::string groupName, EffectParams params, EffectFn fun)
    {
        params.globalSize = 0;
        params.offsetGlobal = 0;

        for (Light *light : lights)
        {
            params.globalSize += (*light)[groupName].size();
        }

        for (Light *light : lights)
        {
            if (!(*light).applyFunctionToAllGroups(groupName, params, fun))
                return false;
            params.offsetGlobal += (*light)[groupName].size();
        }

        return true;
    }

    bool applyFunctionToLights(LightGroup &group, std::string groupName, EffectParams params, EffectFn fun)
    {
        params.globalSize = group.getGlobalGroupSize(groupName);
        params.offsetGlobal = 0;
        auto &lights = group();

        // for (Light *light : lights)
        // {
        //     params.globalSize += (*light)[groupName].size();
        //     // light->print();
        // }

        for (Light *light : lights)
        {
            if (!(*light).applyFunctionToAllGroups(groupName, params, fun))
                return false;
            params.offsetGlobal += (*light)[groupName].size();
        }

        return true;
    }

};
