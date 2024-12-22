#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include "Universe.h"
#include "Utils.h"

using namespace DMX;

int main()
{
    Universe u1(1);
    Universe u2(2);
    Light::Group rgb("RGB", {R, G, B});
    Light light("LedBar", rgb, 40);
    Light par("Par", {R, G, B, Dimmer, Strobo});
    par.addGroup(rgb);

    u1.add(par);
    u1.add(par, 10);
    u1.add(par, 15);
    u1.add(par, 20);

    u2.add(light);
    u2.add(light);

    EffectFn globalRedBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        v[0] = (int)(std::sin(M_PI * ((globalId + params.tick) % params.globalSize) / params.globalSize) * 255);
        v[2] = (int)(std::sin(M_PI * ((globalId + params.tick + params.globalSize / 2) % params.globalSize) / params.globalSize) * 255);
    };

    EffectFn coloriseLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    };

    EffectParams params;
    applyFunctionToLights({u1[0], u1[1], u1[2], u1[3]}, "RGB", params, globalRedBlueLambda);
    applyFunctionToLights({u1[0], u1[1], u1[2], u1[3]}, "RGB", params, coloriseLambda);

    u1.print();
    std::cout << std::endl;

    // tole pa segfaulta
    for (int i = 0; i < 100; i++)
    {
        params.tick = i;
        applyFunctionToLights({u1[0], u1[1], u1[2], u1[3]}, "RGB", params, globalRedBlueLambda);
        applyFunctionToLights({u1[0], u1[1], u1[2], u1[3]}, "RGB", params, coloriseLambda);
        std::cout << "  ";
        applyFunctionToLights({u2[0], u2[1]}, "RGB", params, globalRedBlueLambda);
        applyFunctionToLights({u2[0], u2[1]}, "RGB", params, coloriseLambda);

        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "\r";
    }

    // applyFunctionToLights({u1[0], u1[1], u1[2], u1[3]}, "RGB", EffectParams(), globalRedBlueLambda);
    // applyFunctionToLights({u1[0], u1[1], u1[2], u1[3]}, "RGB", EffectParams(), coloriseLambda);
    // std::cout << std::endl;
    // applyFunctionToLights({u2[0], u2[1]}, "RGB", EffectParams(), globalRedBlueLambda);
    // applyFunctionToLights({u2[0], u2[1]}, "RGB", EffectParams(), coloriseLambda);
    // std::cout << std::endl;

    // u1.print();
    // u2.print();

    // Universe memory(1);
    // Universe memory2(2);
    // Light::Group rgb("RGB", {R, G, B});
    // Light light("LedBar", rgb, 20);
    // Light light2("LedBar2", rgb, 30);
    // Light light3("LedBar3", rgb, 40);

    // if (!memory.add(light))
    // {
    //     return -1;
    // }

    // if (!memory.add(light2))
    // {
    //     return -1;
    // }

    // if (!memory2.add(light3))
    // {
    //     return -1;
    // }

    // // for (int i = 0; i < 10; i++)
    // // {
    // //     Light temp("LedBar4", rgb, 1);
    // //     memory.add(temp);
    // // }

    // EffectFn redBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     v[0] = (int)(std::sin(M_PI * ((params.localId + params.tick) % params.localSize) / params.localSize) * 255);
    //     v[2] = (int)(std::sin(M_PI * ((params.localId + params.tick + params.localSize / 2) % params.localSize) / params.localSize) * 255);
    // };

    // EffectFn coloriseLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    // };

    // EffectFn globalRedBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // {
    //     auto &v = values[params.localId];
    //     int globalId = params.offsetGlobal + params.localId;
    //     v[0] = (int)(std::sin(2 * M_PI * ((globalId + params.tick) % params.globalSize) / params.globalSize) * 255);
    //     v[2] = (int)(std::sin(2 * M_PI * ((globalId + params.tick + params.globalSize / 2) % params.globalSize) / params.globalSize) * 255);
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

    // // EffectFn globalNeki = [](EffectParams &params, std::vector<std::vector<int>> &values)
    // // {
    // //     auto &v = values[params.localId];
    // //     int globalId = params.offsetGlobal + params.localId;
    // //     if (globalId < params.globalSize / 2)
    // //     {
    // //         if ((globalId + (params.tick % 10)) % 10 < 5)
    // //         {
    // //             ((EffectFn)params.other)(params, values);
    // //         }
    // //         else
    // //         {
    // //             v = {0, 0, 0};
    // //         }
    // //     }
    // //     else
    // //     {
    // //         if ((globalId - (params.tick % 10)) % 10 >= 5)
    // //         {
    // //             ((EffectFn)params.other)(params, values);
    // //         }
    // //         else
    // //         {
    // //             v = {0, 0, 0};
    // //         }
    // //     }
    // // };

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

    // EffectParams params(10);
    // params.other = (void *)globalRedBlueLambda;
    // // light2.applyFunctionToAllGroups("RGB", params, redBlueLambda);
    // for (int i = 0; i < 100; i++)
    // {
    //     params.tick = i;
    //     applyFunctionToLights({memory[0], memory[1], memory2[0]}, "RGB", params, globalSegments_Int);
    //     applyFunctionToLights({memory[0], memory[1], memory2[0]}, "RGB", params, globalRedBlueLambda_Col);
    //     applyFunctionToLights({memory[0], memory[1], memory2[0]}, "RGB", params, coloriseLambda);
    //     // memory[0].applyFunctionToAllGroups("RGB", params, redBlueLambda);
    //     // memory[0].applyFunctionToAllGroups("RGB", params, coloriseLambda);

    //     // light2.applyFunctionToAllGroups("RGB", params, hueShiftLambda);
    //     // light2.applyFunctionToAllGroups("RGB", params, coloriseLambda);

    //     // light3.setGroup("RGB", i % 40, Utils::hsvToRgb({(float)fmod((float)i * 5, 360.0f), 1.0, 1.0}));
    //     // light3.applyFunctionToAllGroups("RGB", params, lowerIntesityLambda);
    //     // light3.applyFunctionToAllGroups("RGB", params, coloriseLambda);
    //     std::cout.flush();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //     std::cout << "\r";
    // }
    // std::cout << std::endl;
    // // memory[memory.numLights() - 1].setGroup("RGB", 0, {255, 0, 0});

    // memory.print();
    // memory2.print();
}