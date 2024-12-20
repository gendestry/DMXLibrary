#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace Utils
{

    std::vector<uint8_t> toBytes(int numBytes, int number)
    {
        std::vector<uint8_t> bytes(numBytes);
        for (int i = 0; i < numBytes; i++)
        {
            bytes[numBytes - i - 1] = (number >> (i * 8)) & 0xFF;
        }
        return bytes;
    }

    int toNumber(std::vector<uint8_t> bytes)
    {
        int number = 0;
        int size = bytes.size();
        for (int i = 0; i < bytes.size(); i++)
        {
            number |= bytes[size - i - 1] << (i * 8);
        }
        return number;
    }

    std::string padByte(int num, int pad)
    {
        std::string str = std::to_string(num);
        int len = str.length();
        if (len < pad)
        {
            str.insert(0, pad - len, ' ');
        }
        return str;
    }

    std::string colorByRGB(int r, int g, int b, bool fg)
    {
        std::stringstream ss;
        ss << "\x1B[" << (fg ? 3 : 4) << "8;2;" << r << ";" << g << ";" << b << "m";
        return ss.str();
    }

    std::vector<float> linearInterpolation(float start, float end, int n)
    {
        std::vector<float> values(n);

        float step = (end - start) / (n - 1);
        for (int i = 0; i < n; i++)
        {
            values[i] = start + i * step;
        }

        return values;
    }

    std::vector<float> rgbToHsv(std::vector<int> rgb)
    {
        // Normalize RGB to [0, 1]
        float r = rgb[0] / 255.0f;
        float g = rgb[1] / 255.0f;
        float b = rgb[2] / 255.0f;

        // Find min and max values of R, G, B
        float maxVal = std::max({r, g, b});
        float minVal = std::min({r, g, b});
        float delta = maxVal - minVal;

        // Compute Hue
        float h = 0.0f;
        if (delta > 0.00001f)
        { // Avoid division by zero
            if (maxVal == r)
            {
                h = 60.0f * (fmod(((g - b) / delta), 6.0f));
            }
            else if (maxVal == g)
            {
                h = 60.0f * (((b - r) / delta) + 2.0f);
            }
            else if (maxVal == b)
            {
                h = 60.0f * (((r - g) / delta) + 4.0f);
            }
        }
        if (h < 0.0f)
            h += 360.0f; // Ensure hue is positive

        // Compute Saturation
        float s = (maxVal > 0.00001f) ? (delta / maxVal) : 0.0f;

        // Compute Value
        float v = maxVal;

        return {h, s, v};
    }

    std::vector<int> hsvToRgb(std::vector<float> hsv)
    {
        float h = hsv[0]; // Hue in degrees
        float s = hsv[1]; // Saturation in [0, 1]
        float v = hsv[2]; // Value in [0, 1]

        float r = 0.0f, g = 0.0f, b = 0.0f;

        int hi = static_cast<int>(std::floor(h / 60.0f)) % 6; // Sector of the color wheel (0-5)
        float f = (h / 60.0f) - hi;                           // Fractional part of h/60
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);

        switch (hi)
        {
        case 0:
            r = v, g = t, b = p;
            break;
        case 1:
            r = q, g = v, b = p;
            break;
        case 2:
            r = p, g = v, b = t;
            break;
        case 3:
            r = p, g = q, b = v;
            break;
        case 4:
            r = t, g = p, b = v;
            break;
        case 5:
            r = v, g = p, b = q;
            break;
        }

        // Convert to 0-255 range
        return {static_cast<int>(r * 255.0f),
                static_cast<int>(g * 255.0f),
                static_cast<int>(b * 255.0f)};
    }

};
