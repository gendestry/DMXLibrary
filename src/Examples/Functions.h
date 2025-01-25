#pragma once
#include <iostream>
#include <cmath>
#include <memory>
#include "../DMX/Universe.h"
#include "../Effects/LightGroup.h"
#include "../Effects/Effect.h"
#include "../Utils/Utils.h"

struct ParamsPtr
{
    float intensity;
    int numSegments;
    int segmentSize;
    int color[3];
};

void intensityMaster(Effect::EffectParams &params, std::vector<std::vector<int>> &values)
{
    auto &v = values[params.localId];
    ParamsPtr *ptr = (ParamsPtr *)params.other;
    float intensity = ptr->intensity;
    int value = (int)(intensity * 255);
    std::for_each(v.begin(), v.end(), [&value](int &i)
                  { i = std::max(i, value); });
};
Effect::EffectFn intensityOddEven = [](Effect::EffectParams &params, std::vector<std::vector<int>> &values)
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

Effect::EffectFn intensitySnake = [](Effect::EffectParams &params, std::vector<std::vector<int>> &values)
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

Effect::EffectFn colorGradient = [](Effect::EffectParams &params, std::vector<std::vector<int>> &values)
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

Effect::EffectFn colorRedBlue = [](Effect::EffectParams &params, std::vector<std::vector<int>> &values)
{
    auto &v = values[params.localId];
    int globalId = params.offsetGlobal + params.localId;
    int red = (std::sin(M_PI * ((globalId + params.step) % params.globalSize) / params.globalSize)) * 255;
    int blue = (std::sin(M_PI * ((globalId + params.step + params.globalSize / 2) % params.globalSize) / params.globalSize)) * 255;
    auto hsv = Utils::rgbToHsv({red, 0, blue});
    hsv[2] = v[0] / 255.f;
    v = Utils::hsvToRgb(hsv);
};

Effect::EffectFn colorSingleColor = [](Effect::EffectParams &params, std::vector<std::vector<int>> &values)
{
    auto &v = values[params.localId];
    int globalId = params.offsetGlobal + params.localId;
    ParamsPtr *ptr = (ParamsPtr *)params.other;
    auto &other = ptr->color;
    auto color = Utils::rgbToHsv({other[0], other[1], other[2]});
    color[2] = v[0] / 255.f;
    v = Utils::hsvToRgb(color);
};

Effect::EffectFn coloriseLambda = [](Effect::EffectParams &params, std::vector<std::vector<int>> &values)
{
    auto &v = values[params.localId];
    std::cout << Utils::colorByRGB(v[0], v[1], v[2], false) << "  " << colorReset;
};
