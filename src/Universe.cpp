#include "Universe.h"
#include <iostream>
#include <cmath>

#include "Utils.h"

namespace DMX
{

    bool applyFunctionToLights(std::vector<std::reference_wrapper<Light>> lights, std::string group, EffectParams params, EffectFn fun)
    {
        params.globalSize = 0;
        params.offsetGlobal = 0;
        for (auto &light : lights)
        {
            params.globalSize += light.get()[group].size();
        }

        for (auto &light : lights)
        {
            if (!light.get().applyFunctionToAllGroups(group, params, fun))
                return false;
            params.offsetGlobal += light.get()[group].size();
        }
        return true;
    }

    std::string Universe::nextColor(int index) const
    {
        std::vector<float> colorVecHsv = {110.0f, 0.5f, 0.8f};
        colorVecHsv[0] = fmod(colorVecHsv[0] + (bytesPatched[index] * 65), 360.0f);
        auto colorVec = Utils::hsvToRgb(colorVecHsv);
        return Utils::colorByRGB(colorVec[0], colorVec[1], colorVec[2], true);
    }

    void Universe::fillBytesPatched(int start, int end)
    {
        static unsigned int num = 1;
        for (int i = start; i < end; i++)
        {
            bytesPatched[i] = num;
        }

        num++;
    }

    bool Universe::add(Light &fragment, int start)
    {
        if (lights.empty())
        {
            int size = fragment.getSize();
            if (size <= MAX_SIZE)
            {
                unsigned int startOffset = start == -1 ? 0 : start;
                fragment.start = startOffset;
                fragment.m_bytes = &m_bytes[startOffset];
                lights.emplace_back(fragment);

                fillBytesPatched(startOffset, startOffset + size);
                return true;
            }
            else
            {
                std::cout << "Segment exceeds max size" << std::endl;
                return false;
            }
        }

        // insert next
        if (start == -1)
        {
            auto &frag = lights[lights.size() - 1];

            // if segment fits at end
            if (frag.start + frag.getSize() + fragment.getSize() <= MAX_SIZE)
            {
                fragment.start = frag.start + frag.getSize();
                fragment.m_bytes = &m_bytes[fragment.start];
                lights.push_back(fragment);
                fillBytesPatched(fragment.start, fragment.start + fragment.getSize());
                return true;
            }
        }
        else
        {
            unsigned int size = fragment.getSize();
            for (int i = 0; i < lights.size(); i++)
            {
                auto &current = lights[i];

                // not last segment
                if (i < lights.size() - 1)
                {
                    auto &next = lights[i + 1];
                    if (start >= current.start && start < current.start + current.getSize())
                    {
                        std::cout << "Segment within bounds" << std::endl;
                        return false;
                    }
                    // same for next
                    if (start >= next.start && start < next.start + next.getSize())
                    {
                        std::cout << "Segment within bounds" << std::endl;
                        return false;
                    }
                    if (current.start + current.getSize() <= start && start + size <= next.start)
                    {
                        fragment.start = start;
                        fragment.m_bytes = &m_bytes[fragment.start];
                        lights.insert(lights.begin() + i + 1, fragment);
                        fillBytesPatched(start, start + size);
                        return true;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    // check if segment within current segment bounds
                    if (start >= current.start && start < current.start + current.getSize())
                    {
                        std::cout << "Segment within bounds 1" << std::endl;
                        return false;
                    }

                    if (start >= MAX_SIZE)
                    {
                        std::cout << "Segment start is out of bounds 2" << std::endl;
                        return false;
                    }

                    if (start + size <= MAX_SIZE)
                    {
                        fragment.start = start;
                        fragment.m_bytes = &m_bytes[fragment.start];
                        lights.push_back(fragment);
                        fillBytesPatched(start, start + size);
                        return true;
                    }
                    else
                    {
                        std::cout << "Segment exceeds max size" << std::endl;
                        return false;
                    }
                }
            }
        }

        return false;
    }

    Light &Universe::operator[](int index)
    {
        if (index < 0 || index >= lights.size())
        {
            throw std::runtime_error("Index out of bounds");
        }
        return lights[index];
    }

    std::vector<uint8_t> Universe::getBytes() const
    {
        std::vector<uint8_t> bytes(MAX_SIZE, 0);
        int index = 0;
        for (int i = 0; i < lights.size(); i++)
        {
            const auto &curr = lights[i];
            uint8_t *currBytes = curr.getBytes();

            for (int j = curr.start; j < curr.start + curr.getSize(); j++)
            {
                bytes[j] = currBytes[j - curr.start];
            }
        }

        return bytes;
    }

    void Universe::printFragments() const
    {
        // std::cout << "Segments: " << segments.size() << std::endl;
        constexpr unsigned int printableOffset = 1;
        std::string col;

        for (int i = 0; i < lights.size(); i++)
        {
            const int offset = 0;
            auto &curr = lights[i];
            int cstart = curr.start;
            int cnext = curr.start + curr.getSize();
            int cend = curr.start + curr.getSize() - 1;

            if (cstart != 0 && i == 0)
            {
                printf("\x1B[2m[%3d, %3d]\x1B[3m%s Unpatched %s\n", 0 + offset + printableOffset, cstart - 1 + offset + printableOffset, colorItalic.c_str(), colorReset.c_str());
            }

            col = nextColor(cstart);

            printf("%s[%3d, %3d] %s%s (%d) %s\n", col.c_str(), cstart + offset + printableOffset, cend + offset + printableOffset, colorItalic.c_str(), curr.m_Name.c_str(), curr.getSize(), colorReset.c_str());

            if (i < lights.size() - 1)
            {
                auto &next = lights[i + 1];
                if (curr.start + curr.getSize() != next.start)
                {
                    printf("\x1B[2m[%3d, %3d]\x1B[3m%s Unpatched %s\n", cnext + offset + printableOffset, next.start - 1 + offset + printableOffset, colorItalic.c_str(), colorReset.c_str());
                    continue;
                }
            }
            else
            {
                if (curr.start + curr.getSize() < MAX_SIZE)
                {
                    printf("\x1B[2m[%3d, %3d]\x1B[3m%s Unpatched %s\n", cnext + offset + printableOffset, MAX_SIZE - 1 + offset + printableOffset, colorItalic.c_str(), colorReset.c_str());
                    continue;
                }
            }
        }
    }

    void Universe::printBytes() const
    {
        unsigned int prevByteColor = 0;
        std::vector<float> colorVecHsv = {110.0f, 0.5f, 0.8f};
        std::vector<uint8_t> bytes = getBytes();

        std::string col = nextColor(0);

        int cond = std::min((int)bytes.size(), 16);
        auto printSeperator = [&]()
        {
            for (int i = 0; i < cond; i++)
            {
                std::cout << "----";
                if (i < cond - 1)
                    std::cout << "-";
            }
            std::cout << std::endl;
        };

        std::cout << colorItalic;
        for (int i = 0; i < cond; i++)
        {
            static const std::string hex = "0123456789ABCDEF";
            std::cout << "0x" << hex[i] << "  ";
        }
        std::cout << colorReset << std::endl;

        printSeperator();

        for (int i = 0; i < bytes.size(); i++)
        {
            if (i % 16 == 0 && i != 0)
                std::cout << std::endl;

            if (bytesPatched[i] != 0)
                col = nextColor(i);
            else
                col = colorReset + "\x1B[2m" + "\x1B[3m";

            std::cout << col << Utils::padByte(bytes[i], 3) << colorReset << "  ";
        }
        std::cout << std::endl;

        printSeperator();
    }

    void Universe::print() const
    {
        std::cout << "Universe: " << universeID << ", Number of Lights: " << lights.size() << std::endl;
        std::cout << "Patch list: " << std::endl;
        printFragments();

        std::cout << "Bytes: " << std::endl;
        printBytes();
    }

};
