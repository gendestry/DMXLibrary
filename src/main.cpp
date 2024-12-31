#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <memory>
#include "Universe.h"
#include "Utils.h"
#include "LightGroup.h"

using namespace DMX;

#include "Effect.h"

int main()
{

    struct ParamsPtr
    {
        float intensity;
        int numSegments;
        int segmentSize;
        int color[3];
    };

    EffectFn intensityMaster = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        ParamsPtr *ptr = (ParamsPtr *)params.other;
        float intensity = ptr->intensity;
        int value = (int)(intensity * 255);
        std::for_each(v.begin(), v.end(), [&value](int &i)
                      { i = std::max(i, value); });
    };

    EffectFn intensityOddEven = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        if (globalId % 2)
            return;
        float intensity = 1.f;
        int value = (int)(intensity * 255);
        std::for_each(v.begin(), v.end(), [&value](int &i)
                      { i = std::max(i, value); });
    };

    EffectFn intensitySnake = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        ParamsPtr *ptr = (ParamsPtr *)params.other;
        int numSegments = ptr->numSegments;
        int segmentSize = ptr->segmentSize;
        Utils::SegmentOptions opt = Utils::SegmentOptions::Sin;
        int index = params.offsetGlobal + params.localId;
        int tick = params.tick;
        int n = params.globalSize;
        auto &v = values[params.localId];

        std::vector<float> segs;
        segs = Utils::getSegments(numSegments, segmentSize, n, tick, opt);

        int val = (int)(255.f * segs[index]);
        v = {val, val, val};
    };

    EffectFn colorGradient = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        int index = params.offsetGlobal + params.localId;
        int tick = params.tick;
        int n = params.globalSize;
        auto &v = values[params.localId];
        auto gradient = Utils::getGradient(n, {{100, 180, 200}, {180, 80, 10}}, {0.5f, 0.5f})[(index + tick) % n];
        auto hsv = Utils::rgbToHsv(gradient);
        hsv[2] = v[0] / 255.f;
        v = Utils::hsvToRgb(hsv);
    };

    EffectFn colorRedBlue = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        int red = (std::sin(M_PI * ((globalId + params.tick) % params.globalSize) / params.globalSize)) * 255;
        int blue = (std::sin(M_PI * ((globalId + params.tick + params.globalSize / 2) % params.globalSize) / params.globalSize)) * 255;
        auto hsv = Utils::rgbToHsv({red, 0, blue});
        hsv[2] = v[0] / 255.f;
        v = Utils::hsvToRgb(hsv);
    };

    EffectFn colorSingleColor = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        ParamsPtr *ptr = (ParamsPtr *)params.other;
        auto &other = ptr->color;
        auto color = Utils::rgbToHsv({other[0], other[1], other[2]});
        color[2] = v[0] / 255.f;
        v = Utils::hsvToRgb(color);
    };

    EffectFn coloriseLambda = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
    };

    Light::Group rgb("RGB", {R, G, B});
    Light light("Pixel", rgb, 1);
    Light par("Par", {R, G, B, Dimmer, Strobo});
    par.addGroup(rgb);

    Universe u1(1);
    Universe u2(2);

    for (int i = 0; i < 30; i++)
    {
        u2.add(par);
    }
    for (int i = 0; i < 60; i++)
    {
        u1.add(light);
    }

    LightGroup ledbar, pars, all;
    ledbar += u1["Pixel"];
    pars += u2["Par"];
    all += u2[(LightsInterval){"Par", 0, 14}];
    all += ledbar;
    all += u2[(LightsInterval){"Par", 15, 29}];

    IntensityEffect iSnakeFx(intensitySnake);
    IntensityEffect iMasterFx(intensityMaster);
    IntensityEffect iOddEvenFx(intensityOddEven);
    ColorEffect cRedBlueFx(colorRedBlue);
    ColorEffect cGradientFx(colorGradient);
    ColorEffect cSingleColorFx(colorSingleColor);
    OtherEffect oColoriseFx(coloriseLambda);

    FX fx(ledbar);
    fx.add<Intensity>(&iSnakeFx);
    fx.add<Color>(&cSingleColorFx);

    FX fx2(pars);
    fx2.add<Intensity>(&iMasterFx);
    fx2.add<Intensity>(&iOddEvenFx);
    fx2.add<Color>(&cRedBlueFx);

    FX fx3(all);
    fx3.add<Other>(&oColoriseFx);

    EffectParams params;
    ParamsPtr ptr = {0.5f, 4, 10, {80, 200, 140}};
    params.other = (void *)&ptr;
    for (int i = 0; i < 100; i++)
    {
        params.tick = i;
        fx.apply("RGB", params);
        fx2.apply("RGB", params);
        fx3.apply("RGB", params);
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "\r";
    }

    return 0;
}
