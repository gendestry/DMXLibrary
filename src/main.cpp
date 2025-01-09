#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <memory>
#include "DMX/Universe.h"
#include "Effects/LightGroup.h"
#include "Effects/Effect.h"
#include "Utils/Utils.h"

using namespace DMX;
using namespace Effect;

int main()
{

    // auto oldMillis = timeSinceEpochMillisec();
    // int bpm = 60;
    // float freq = bpm / 60.f * 1000.f;
    // while (true)
    // {
    //     auto millis = timeSinceEpochMillisec();
    //     auto dt = millis - oldMillis;
    //     if (dt > freq)
    //     {
    //         std::cout << dt << std::endl;
    //         oldMillis = millis;
    //     }
    // }
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
        Utils::SegmentOptions opt = Utils::SegmentOptions::Saw;
        int index = params.offsetGlobal + params.localId;
        int step = params.step;
        int n = params.globalSize;
        auto &v = values[params.localId];

        std::vector<float> segs;
        segs = Utils::getSegments(numSegments, segmentSize, n, step, opt);

        int val = (int)(255.f * segs[index]);
        v = {val, val, val};
    };

    EffectFn colorGradient = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        int index = params.offsetGlobal + params.localId;
        int step = params.step;
        int n = params.globalSize;
        auto &v = values[params.localId];
        auto gradient = Utils::getGradient(n, {{255, 0, 0}, {255, 255, 0}, {70, 100, 255}}, {0.4, 0.2f, 0.4f})[(index + step) % n];
        auto hsv = Utils::rgbToHsv(gradient);
        hsv[2] = v[0] / 255.f;
        v = Utils::hsvToRgb(hsv);
    };

    EffectFn colorRedBlue = [](EffectParams &params, std::vector<std::vector<int>> &values)
    {
        auto &v = values[params.localId];
        int globalId = params.offsetGlobal + params.localId;
        int red = (std::sin(M_PI * ((globalId + params.step) % params.globalSize) / params.globalSize)) * 255;
        int blue = (std::sin(M_PI * ((globalId + params.step + params.globalSize / 2) % params.globalSize) / params.globalSize)) * 255;
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
    Light pixel("Pixel", rgb, 1);
    Light par("Par", {R, G, B, Dimmer, Strobo});
    par.addGroup(rgb);

    Universe u1(1);
    Universe u2(2);

    u1.addMultiple(pixel, 60);
    u2.addMultiple(par, 30);

    Effect::LightGroup ledbar, pars, all;
    ledbar += u1["Pixel"];
    pars += u2["Par"];
    // all += u2[(LightsInterval){"Par", 0, 14}];
    all += pars;
    all += ledbar;
    // all += u2[(LightsInterval){"Par", 15, 29}];

    using namespace Effect;
    IntensityEffect iSnakeFx(intensitySnake, 60);
    IntensityEffect iMasterFx(intensityMaster);
    IntensityEffect iOddEvenFx(intensityOddEven);
    ColorEffect cRedBlueFx(colorRedBlue, 60);
    ColorEffect cGradientFx(colorGradient, 60);
    ColorEffect cSingleColorFx(colorSingleColor);
    OtherEffect oColoriseFx(coloriseLambda, 120);

    FX fx;
    fx.add<Intensity>(&iMasterFx);
    fx.add<Color>(&cRedBlueFx);

    FX fx2;
    fx2.add<Intensity>(&iSnakeFx);
    fx2.add<Color>(&cRedBlueFx);

    // fx2.add<Color>(&cSingleColorFx);

    FX fx3;
    fx3.add<Other>(&oColoriseFx);

    EffectParams params;
    ParamsPtr ptr = {0.8f, 1, 7, {80, 200, 140}};
    params.other = (void *)&ptr;
    // // get current time millis
    for (int i = 0;; i++)
    {
        params.tick = i;
        fx.apply(pars, "RGB", params);
        fx2.apply(ledbar, "RGB", params);
        fx3.apply(all, "RGB", params);
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::cout << "\r";
    }
    std::cout << std::endl;
    return 0;
}
