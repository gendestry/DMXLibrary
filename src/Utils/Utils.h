#pragma once
#include <vector>
#include <string>

namespace Utils
{

    std::vector<uint8_t> toBytes(int numBytes, int number);
    int toNumber(std::vector<uint8_t> bytes);
    std::string padByte(int num, int pad);
    std::string colorByRGB(int r, int g, int b, bool fg);
    std::vector<float> linearInterpolation(float start, float end, int n);

    std::vector<float> rgbToHsv(std::vector<int> rgb);
    std::vector<int> hsvToRgb(std::vector<float> hsv);

    enum SegmentOptions
    {
        Binary,
        Sin,
        Saw,
        InvSaw
    };

    std::vector<float> getSegments(int numSegments, int segmentSize, int totalSize, int offset, SegmentOptions opt = SegmentOptions::Sin);
    std::vector<std::vector<int>> getGradient(int numElements, std::vector<std::vector<int>> colors, std::vector<float> percentages = {});

    uint64_t micros();
};
