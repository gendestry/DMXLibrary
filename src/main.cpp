#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include "Universe.h"
#include "Utils.h"
#include "LightGroup.h"

using namespace DMX;

int main()
{

    EffectFn globalRedBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        v[0] = (int)(std::sin(M_PI * ((globalId + params.tick) % params.globalSize) / params.globalSize) * 255);
        v[1] = 0;
        v[2] = (int)(std::sin(M_PI * ((globalId + params.tick + params.globalSize / 2) % params.globalSize) / params.globalSize) * 255);
    };

    EffectFn coloriseLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    };

    EffectFn globalSnake = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        float globalId = (float)params.offsetGlobal + params.localId;
        if (params.tick % params.globalSize == globalId)
        {
            auto rgb = Utils::hsvToRgb({(globalId / params.globalSize * 359.0f), 1.0, 1.0});
            v[0] = rgb[0];
            v[1] = rgb[1];
            v[2] = rgb[2];
        }
        else
        {
            auto hsv = Utils::rgbToHsv(v);
            hsv[2] *= 0.9;
            auto rgb = Utils::hsvToRgb(hsv);
            v[0] = rgb[0];
            v[1] = rgb[1];
            v[2] = rgb[2];
        }
    };

    EffectParams params;

    Light::Group rgb("RGB", {R, G, B});
    Light light("LedBar", rgb, 1);
    Light par("Par", {R, G, B, Dimmer, Strobo});
    par.addGroup(rgb);

    Universe u1(1);
    Universe u2(2);

    // ce se add klice 25x,, crasha-----
    // for (int i = 0; i < 25; i++)
    // {
    //     u1.add(light);
    // }

    // for (int i = 0; i < 25; i++)
    // {
    //     u1.add(par);
    // }

    // for (int i = 0; i < 25; i++)
    // {
    //     Light l("led", rgb, 1);
    //     u1.add(l);
    // }

    // u1.print();
    // light.print();

    for (int i = 0; i < 17; i++)
    {
        u1.add(light);
    }
    for (int i = 0; i < 7; i++)
    {
        u1.add(par);
    }

    for (int i = 0; i < 17; i++)
    {
        u2.add(light);
    }
    for (int i = 0; i < 7; i++)
    {
        u2.add(par);
    }

    LightGroup ledbars;
    LightGroup pars;
    LightGroup both;

    ledbars += u1["LedBar"];
    pars += u1["Par"];
    both += ledbars;
    both += pars;

    // std::cout << "Group size: " << ledbars.size() << std::endl;
    // std::cout << "Group size: " << pars.size() << std::endl;
    // std::cout << "Group size: " << both.size() << std::endl;

    // applyFunctionToLights(both, "RGB", params, [](EffectParams &params, std::vector<std::vector<int>> &values)
    //                       { values[params.localId] = {180, 110, 30}; });
    // applyFunctionToLights(both, "RGB", params, coloriseLambda);

    // std::cout.flush();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // std::cout << "\r";

    // applyFunctionToLights(ledbars, "RGB", params, [](EffectParams &params, std::vector<std::vector<int>> &values)
    //                       { values[params.localId] = {30, 180, 110}; });
    // applyFunctionToLights(pars, "RGB", params, [](EffectParams &params, std::vector<std::vector<int>> &values)
    //                       { values[params.localId] = {30, 110, 180}; });
    // applyFunctionToLights(both, "RGB", params, coloriseLambda);

    // std::cout.flush();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // std::cout << "\r";

    // for (int i = 0; i < 50; i++)
    // {
    //     params.tick = i;
    //     applyFunctionToLights(pars, "RGB", params, globalRedBlueLambda);
    //     applyFunctionToLights(ledbars, "RGB", params, globalSnake);
    //     applyFunctionToLights(both, "RGB", params, coloriseLambda);

    //     std::cout.flush();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //     std::cout << "\r";
    // }

    // std::cout << std::endl;

    // print all lights in u1
    // for (int i = 0; i < u1.numLights(); i++)
    // {
    //     u1[i].print();
    // }

    // u1.print();

    // applyFunctionToLights(group1, "RGB", params, coloriseLambda);

    // EffectParams params;
    // light.applyFunctionToAllGroups("RGB", params, [](EffectParams &params, std::vector<std::vector<int>> &values)
    //                                { values[params.localId] = {30, 190, 110}; });
    // light.print();
    // Universe u1(1);
    // Universe u2(2);
    // Light::Group rgb("RGB", {R, G, B});
    // Light light("LedBar", rgb, 40);
    // Light par("Par", {R, G, B, Dimmer, Strobo});
    // par.addGroup(rgb);

    // u1.add(par);
    // u1.add(par, 10);
    // u1.add(par, 15);
    // u1.add(par, 20);

    // u2.add(light);
    // u2.add(light);

    // EffectFn globalRedBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     int globalId = params.offsetGlobal + params.localId;
    //     v[0] = (int)(std::sin(M_PI * ((globalId + params.tick) % params.globalSize) / params.globalSize) * 255);
    //     v[1] = 0;
    //     v[2] = (int)(std::sin(M_PI * ((globalId + params.tick + params.globalSize / 2) % params.globalSize) / params.globalSize) * 255);
    // };

    // EffectFn globalSnake = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     float globalId = (float)params.offsetGlobal + params.localId;
    //     if (params.tick % params.globalSize == globalId)
    //     {
    //         auto rgb = Utils::hsvToRgb({(globalId / params.globalSize * 359.0f), 1.0, 1.0});
    //         v[0] = rgb[0];
    //         v[1] = rgb[1];
    //         v[2] = rgb[2];
    //     }
    //     else
    //     {
    //         auto hsv = Utils::rgbToHsv(v);
    //         hsv[2] *= 0.9;
    //         auto rgb = Utils::hsvToRgb(hsv);
    //         v[0] = rgb[0];
    //         v[1] = rgb[1];
    //         v[2] = rgb[2];
    //     }
    // };

    // EffectFn coloriseLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    // };

    // EffectParams params;
    // u1[0]["RGB"][0].setValues({255, 0, 0});
    // u1[1]["RGB"][0].setValues({255, 0, 0});
    // u1[2]["RGB"][0].setValues({255, 0, 0});
    // u1[3]["RGB"][0].setValues({255, 255, 255});
    // u2[0].applyFunctionToAllGroups("RGB", params, [](EffectParams &params, std::vector<std::vector<int>> &values)
    //                                { values[params.localId] = {30, 190, 110}; });
    // u2[1].applyFunctionToAllGroups("RGB", params, [](EffectParams &params, std::vector<std::vector<int>> &values)
    //                                { values[params.localId] = {30, 110, 190}; });

    // applyFunctionToLights({u1[0], u1[1], u1[2], u1[3], u2[0], u2[1]}, "RGB", params, coloriseLambda);
    // std::cout.flush();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // std::cout << "\r";

    // for (int i = 0; i < 200; i++)
    // {
    //     params.tick = i;
    //     applyFunctionToLights({u1[0], u1[1], u1[2], u2[0]}, "RGB", params, globalRedBlueLambda);
    //     applyFunctionToLights({u1[3], u2[1]}, "RGB", params, globalSnake);
    //     applyFunctionToLights({u1[0], u1[1], u1[2], u1[3], u2[0], u2[1]}, "RGB", params, coloriseLambda);

    //     std::cout.flush();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //     std::cout << "\r";
    // }

    // std::cout << std::endl;

    // u1.print();
    // u2.print();

    // EffectFn redBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     v[0] = (int)(std::sin(M_PI * ((params.localId + params.tick) % params.localSize) / params.localSize) * 255);
    //     v[2] = (int)(std::sin(M_PI * ((params.localId + params.tick + params.localSize / 2) % params.localSize) / params.localSize) * 255);
    // };

    // EffectFn globalSnake = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     float globalId = (float)params.offsetGlobal + params.localId;
    //     if (params.tick % params.globalSize == globalId)
    //     {
    //         auto rgb = Utils::hsvToRgb({(globalId / params.globalSize * 359.0f), 1.0, 1.0});
    //         v[0] = rgb[0];
    //         v[1] = rgb[1];
    //         v[2] = rgb[2];
    //     }
    //     else
    //     {
    //         auto hsv = Utils::rgbToHsv(v);
    //         hsv[2] *= 0.9;
    //         auto rgb = Utils::hsvToRgb(hsv);
    //         v[0] = rgb[0];
    //         v[1] = rgb[1];
    //         v[2] = rgb[2];
    //     }
    // };

    // EffectFn globalNeki = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     int globalId = params.offsetGlobal + params.localId;
    //     if (globalId < params.globalSize / 2)
    //     {
    //         if ((globalId + (params.tick % 10)) % 10 < 5)
    //         {
    //             ((EffectFn)params.other)(params, values);
    //         }
    //         else
    //         {
    //             v = {0, 0, 0};
    //         }
    //     }
    //     else
    //     {
    //         if ((globalId - (params.tick % 10)) % 10 >= 5)
    //         {
    //             ((EffectFn)params.other)(params, values);
    //         }
    //         else
    //         {
    //             v = {0, 0, 0};
    //         }
    //     }
    // };

    // EffectFn globalSegments_Int = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     int globalId = params.offsetGlobal + params.localId;
    //     if (globalId < params.globalSize / 2)
    //     {
    //         if ((globalId + (params.tick % 10)) % 10 < 5)
    //         {
    //             v = {255, 255, 255};
    //         }
    //         else
    //         {
    //             v = {0, 0, 0};
    //         }
    //     }
    //     else
    //     {
    //         if ((globalId - (params.tick % 10)) % 10 >= 5)
    //         {
    //             v = {255, 255, 255};
    //         }
    //         else
    //         {
    //             v = {0, 0, 0};
    //         }
    //     }
    // };

    // EffectFn globalRedBlueLambda_Col = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     int globalId = params.offsetGlobal + params.localId;
    //     v[0] *= (std::sin(M_PI * ((globalId + params.tick) % params.globalSize) / params.globalSize));
    //     v[1] = 0;
    //     v[2] *= (std::sin(M_PI * ((globalId + params.tick + params.globalSize / 2) % params.globalSize) / params.globalSize));
    // };
}
