#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include "Universe.h"
#include "Utils.h"
#include "LightGroup.h"

using namespace DMX;

#include "Effect.h"

int main()
{
    EffectFn intensityFun = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        float intensity = 0.6f;
        int value = (int)(intensity * 255);
        v = {value, value, value};
    };

    EffectFn oddEvenIntensityFun = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        if (globalId % 2)
            return;
        float intensity = 1.f;
        int value = (int)(intensity * 255);
        v = {value, value, value};
    };

    EffectFn snakeIntensityFun = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        // if(params.tick <)
    };

    EffectFn globalRedBlueLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        v[0] *= (std::sin(M_PI * ((globalId + params.tick) % params.globalSize) / params.globalSize));
        v[1] = 0;
        v[2] *= (std::sin(M_PI * ((globalId + params.tick + params.globalSize / 2) % params.globalSize) / params.globalSize));
    };

    EffectFn coloriseLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    };

    EffectFn globalSnake = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int n = params.globalSize;
        const int size = 5;
        int tick = (params.tick - size) % n;
        int i = params.offsetGlobal + params.localId;

        int tail = tick % n;
        int head = (size + tick) % n;
        if (head < tail)
        {
            if (i >= tail || i <= head)
                v = {255, 255, 255};
            else
                v = {0, 0, 0};
        }
        else
        {
            if (i >= tail && i <= head)
                v = {255, 255, 255};
            else
                v = {0, 0, 0};
        }
    };

    Light::Group rgb("RGB", {R, G, B});
    Light light("Pixel", rgb, 1);
    Universe u1(1);

    for (int i = 0; i < 40; i++)
    {
        u1.add(light);
    }

    u1[0].setGroup("RGB", 0, {255, 0, 0});
    u1[1].setGroup("RGB", 0, {255, 0, 0});

    LightGroup ledbar;
    ledbar += u1["Pixel"];

    FX fx(ledbar);
    IntensityEffect ie(globalSnake);
    IntensityEffect ie2(oddEvenIntensityFun);
    ColorEffect ce(globalRedBlueLambda);
    OtherEffect oe(coloriseLambda);
    fx.add<Intensity>(&ie);
    // fx.add<Intensity>(&ie2);
    fx.add<Color>(&ce);
    fx.add<Other>(&oe);

    EffectParams params;
    for (int i = 0; i < 100; i++)
    {
        params.tick = i;
        fx.apply("RGB", params);

        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "\r";
    }
    std::cout << std::endl;

    return 0;

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

    // EffectParams params;

    // Light::Group rgb("RGB", {R, G, B});
    // Light light("LedBar", rgb, 1);
    // Light par("Par", {R, G, B, Dimmer, Strobo});
    // par.addGroup(rgb);

    // Universe u1(1);
    // Universe u2(2);

    // for (int i = 0; i < 30; i++)
    // {
    //     u1.add(light);
    // }
    // for (int i = 0; i < 10; i++)
    // {
    //     u1.add(par);
    // }

    // // for (int i = 0; i < 17; i++)
    // // {
    // //     u2.add(light);
    // // }
    // for (int i = 0; i < 7; i++)
    // {
    //     u2.add(par);
    // }

    // LightGroup ledbars;
    // LightGroup pars;
    // LightGroup both;

    // ledbars += u1["LedBar"];
    // pars += u1["Par"];
    // pars += u2["Par"];
    // both += u1["Par"];
    // both += ledbars;
    // both += u2["Par"];

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

    // for (int i = 0; i < 100; i++)
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
