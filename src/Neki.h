#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include "LightPatch.h"
#include "Utils.h"

#define MAX_SIZE 20

struct Seg
{
    int start;
    int size;
    LightPatch light;

    friend std::ostream &operator<<(std::ostream &os, const Seg &seg)
    {
        // os << "Segment: [" << seg.start << ", " << seg.start + seg.size - 1 << "] Size: " << seg.size;
        os << seg.light;
        return os;
    }
};

struct Manager
{
    std::vector<Seg> segments;

    //     void addLightPatch(LightPatch patch);

    std::vector<uint8_t> byteValue() const
    {
        std::vector<uint8_t> bytes;
        for (const auto &seg : segments)
        {
            auto patchBytes = seg.light.byteValue();
            bytes.insert(bytes.end(), patchBytes.begin(), patchBytes.end());
        }
        return bytes;
    }

    LightPatch &operator[](int index)
    {
        return segments[index].light;
    }

    void printByteValue() const
    {
        std::vector<uint8_t> bytes = byteValue();
        std::cout << "Bytes: " << "\x1B[32m";
        int i = 0;
        for (; i < bytes.size(); i++)
        {
            if (i % 16 == 0)
                std::cout << std::endl;
            std::cout << padByte(bytes[i], 3) << "  ";
        }

        std::cout << "\x1B[0m";
        while (i < 512)
        {
            if (i % 16 == 0)
                std::cout << std::endl;
            std::cout << padByte(0, 3) << "  ";

            i++;
        }
        std::cout << std::endl;
    }

    //     void printLightPatches();
    //     void printByteValue() const;

    bool addSegment(LightPatch light)
    {
        int size = light.patchSize();
        if (segments.empty())
        {
            if (size <= MAX_SIZE)
            {
                segments.push_back({0, size, light});
            }
            else
            {
                std::cout << "Segment exceeds max size" << std::endl;
                return false;
            }
        }
        else
        {
            // find list segment
            auto &seg = segments[segments.size() - 1];

            // if segment fits at end
            if (seg.start + seg.size + size <= MAX_SIZE)
            {
                segments.push_back({seg.start + seg.size, size, light});
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool addSegment(int start, LightPatch light)
    {
        int size = light.patchSize();
        if (segments.size() == 0)
        {
            if (start + size < MAX_SIZE)
            {
                segments.push_back({start, size, light});
                auto &seg = segments[segments.size() - 1];
                // std::cout << seg.start << " " << seg.size << std::endl;
                // std::cout << size << std::endl;
                // std::cout << "Segment added " << segments[segments.size()] << std::endl;
                return true;
            }
            else
            {
                std::cout << "Segment exceeds max size" << std::endl;
                return false;
            }
        }

        for (int i = 0; i < segments.size(); i++)
        {
            auto &current = segments[i];

            // not last segment
            if (i < segments.size() - 1)
            {
                auto &next = segments[i + 1];
                if (current.start + current.size <= start && start + size <= next.start)
                {
                    segments.insert(segments.begin() + i + 1, {start, size, light});
                    return true;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if (start >= MAX_SIZE)
                {
                    std::cout << "Segment start is out of bounds" << std::endl;
                    return false;
                }

                else if (start + size < MAX_SIZE)
                {
                    segments.push_back({start, size, light});
                    return true;
                }
                else
                {
                    std::cout << "Segment exceeds max size" << std::endl;
                    return false;
                }
            }
        }

        return false;
    }

    void printSegments()
    {
        // std::cout << "Segments: " << segments.size() << std::endl;
        for (int i = 0; i < segments.size(); i++)
        {
            auto &curr = segments[i];
            int cstart = curr.start;
            int cnext = curr.start + curr.size;
            int cend = curr.start + curr.size - 1;

            std::cout << "[" << std::setw(3) << (curr.start) << ", " << std::setw(3) << (curr.start + curr.size - 1) << "] " << curr << std::endl;

            if (i < segments.size() - 1)
            {
                auto &next = segments[i + 1];
                if (curr.start + curr.size != next.start)
                {
                    std::cout << "[" << std::setw(3) << (curr.start + curr.size) << ", " << std::setw(3) << (next.start - 1) << "]" << " Empty" << std::endl;
                    continue;
                }
            }
            else
            {
                if (curr.start + curr.size < MAX_SIZE)
                {
                    std::cout << "[" << std::setw(3) << (curr.start + curr.size) << ", " << std::setw(3) << (MAX_SIZE - 1) << "]" << " Empty" << std::endl;
                    continue;
                }
            }
        }
    }
};