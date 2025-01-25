#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <memory>
#include "DMX/Universe.h"
#include "Effects/LightGroup.h"
#include "Effects/Effect.h"
#include "Utils/Utils.h"
#include "Examples/Functions.h"
#include "Engine/Engine.h"

using namespace DMX;
using namespace Effect;

int main()
{
    Light::Group rgb("RGB", {R, G, B});
    Light pixel("Pixel", rgb, 1);
    Light par("Par", {R, G, B, Dimmer, Strobo});
    par.addGroup(rgb);

    Engine::Engine engine;
    engine[1].addMultiple(pixel, 60);
    engine[1].addMultiple(par, 10);
    engine[2].addMultiple(par, 30);

    engine["ledbar"] += engine[1]["Pixel"];
    engine["pars"] += engine[1]["Par"];
    engine["pars"] += engine[2]["Par"];

    engine["all"] += engine["pars"];
    engine["all"] -= engine["ledbar"];

    IntensityEffect iSnakeFx(intensitySnake, 20);
    IntensityEffect iMasterFx(intensityMaster);
    IntensityEffect iOddEvenFx(intensityOddEven);
    ColorEffect cRedBlueFx(colorRedBlue, 60);
    ColorEffect cGradientFx(colorGradient, 30);
    ColorEffect cSingleColorFx(colorSingleColor);
    OtherEffect oColoriseFx(coloriseLambda);

    EffectParams params;
    ParamsPtr ptr = {0.8f, 5, 10, {80, 200, 140}};
    params.other = (void *)&ptr;

    engine.getFX("fx1").add<Intensity>(&iMasterFx);
    engine.getFX("fx1").add<Color>(&cRedBlueFx);

    engine.getFX("fx2").add<Intensity>(&iSnakeFx);
    engine.getFX("fx2").add<Color>(&cSingleColorFx);

    engine.getFX("fx3").add<Other>(&oColoriseFx);

    // // get current time millis
    for (int i = 0; i < 10000; i++)
    {
        params.tick = i;
        engine.getFX("fx1").apply(engine["pars"], "RGB", params);
        engine.getFX("fx2").apply(engine["ledbar"], "RGB", params);
        engine.getFX("fx3").apply(engine["all"], "RGB", params);
        std::cout.flush();
        // std::this_thread::sleep_for(std::chrono::milliseconds((Effect::Effect::minBPM * 1000) / 120));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::cout << "\r";
    }
    std::cout << std::endl;
    engine[1].print();
    engine[2].print();
    return 0;
}
