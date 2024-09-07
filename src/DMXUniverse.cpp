#include "DMXUniverse.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include "Utils.h"

std::vector<uint8_t> DMXUniverse::byteValue() const
{
    std::vector<uint8_t> bytes(512, 0);

    int index = 0;
    for (int i = 0; i < segments.size(); i++)
    {
        auto &curr = segments[i];
        int cstart = curr.start;
        int cnext = curr.start + curr.size;
        int cend = curr.start + curr.size - 1;

        for (int j = cstart; j < cnext; j++)
        {
            auto patchBytes = curr.light.byteValue();
            bytes[j] = patchBytes[j - cstart];
        }
    }
    // for (const auto &seg : segments)
    // {
    //     auto patchBytes = seg.light.byteValue();
    //     bytes.insert(bytes.end(), patchBytes.begin(), patchBytes.end());
    // }
    return bytes;
}

LightPatch &DMXUniverse::operator[](int index)
{
    return segments[index % segments.size()].light;
}

//     void printLightPatches();
//     void printByteValue() const;

bool DMXUniverse::addSegment(LightPatch light)
{
    int size = light.patchSize();
    if (segments.empty())
    {
        if (size <= MAX_SIZE)
        {
            segments.push_back({0, size, light});
            fillBytesPatched(0, size);
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
            auto &newSeg = segments[segments.size() - 1];
            fillBytesPatched(newSeg.start, newSeg.start + newSeg.size);
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool DMXUniverse::addSegment(int start, LightPatch light)
{
    int size = light.patchSize();

    // if we have no segments yet
    if (segments.size() == 0)
    {
        if (start + size < MAX_SIZE)
        {
            segments.push_back({start, size, light});
            fillBytesPatched(start, start + size);
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
        // std::cout << "Current: " << current << std::endl;
        // std::cout << "i: " << i << ", Segment size: " << segments.size() << std::endl;

        // not last segment
        if (i < segments.size() - 1)
        {
            // std::cout << "here" << std::endl;
            auto &next = segments[i + 1];
            // std::cout << "current: " << current.start << ", " << current.size << std::endl;
            // std::cout << "next   : " << next.start << ", " << next.size << std::endl;
            // std::cout << "other  : " << start << ", " << size << std::endl;
            if (start >= current.start && start < current.start + current.size)
            {
                std::cout << "Segment within bounds" << std::endl;
                return false;
            }
            // same for next
            if (start >= next.start && start < next.start + next.size)
            {
                std::cout << "Segment within bounds" << std::endl;
                return false;
            }
            if (current.start + current.size <= start && start + size <= next.start)
            {
                segments.insert(segments.begin() + i + 1, {start, size, light});
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
            // std::cout << current.start << ", " << current.size << std::endl;
            // std::cout << start << ", " << size << std::endl;
            if (start >= current.start && start < current.start + current.size)
            {
                std::cout << "Segment within bounds" << std::endl;
                return false;
            }

            if (start >= MAX_SIZE)
            {
                std::cout << "Segment start is out of bounds" << std::endl;
                return false;
            }

            if (start + size < MAX_SIZE)
            {
                segments.push_back({start, size, light});
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

    return false;
}

void DMXUniverse::fillBytesPatched(int from, int to, bool value)
{
    for (int i = from; i < to; i++)
    {
        isPatched[i] = value;
    }
}

void DMXUniverse::printSegments()
{
    // std::cout << "Segments: " << segments.size() << std::endl;
    for (int i = 0; i < segments.size(); i++)
    {
        const int offset = 0;
        auto &curr = segments[i];
        int cstart = curr.start;
        int cnext = curr.start + curr.size;
        int cend = curr.start + curr.size - 1;

        if (cstart != 0 && i == 0)
        {
            std::cout << "[" << std::setw(3) << 0 + offset << ", " << std::setw(3) << (cstart - 1 + offset) << "]" << " Empty" << std::endl;
        }

        std::cout << "[" << std::setw(3) << (curr.start + offset) << ", " << std::setw(3) << (curr.start + curr.size - 1 + offset) << "] " << curr << std::endl;

        if (i < segments.size() - 1)
        {
            auto &next = segments[i + 1];
            if (curr.start + curr.size != next.start)
            {
                std::cout << "[" << std::setw(3) << (curr.start + curr.size + offset) << ", " << std::setw(3) << (next.start - 1 + offset) << "]" << " Empty" << std::endl;
                continue;
            }
        }
        else
        {
            if (curr.start + curr.size < MAX_SIZE)
            {
                std::cout << "[" << std::setw(3) << (curr.start + curr.size + offset) << ", " << std::setw(3) << (MAX_SIZE - 1 + offset) << "]" << " Empty" << std::endl;
                continue;
            }
        }
    }
}

void DMXUniverse::printByteValue() const
{

    const std::string color = "\x1B[32m";
    const std::string reset = "\x1B[0m";
    std::vector<uint8_t> bytes = byteValue();
    std::cout << "Bytes: ";
    for (int i = 0; i < bytes.size(); i++)
    {
        if (i % 16 == 0)
            std::cout << std::endl;

        std::cout << (isPatched[i] ? color : "") << padByte(bytes[i], 3) << reset << "  ";
    }

    std::cout << std::endl;
}

// #include "DMXUniverse.h"
// #include "Utils.h"

// #include <iostream>

// void DMXUniverse::addLightPatch(LightPatch patch)
// {
//     if (m_Size + patch.size() > 512)
//     {
//         throw std::runtime_error("Patch size exceeds 512 bytes");
//         return;
//     }

//     m_LightPatches.push_back(patch);
//     m_Size += patch.size();
// }

// std::vector<uint8_t> DMXUniverse::byteValue() const
// {
//     std::vector<uint8_t> bytes;
//     for (const auto &patch : m_LightPatches)
//     {
//         auto patchBytes = patch.byteValue();
//         bytes.insert(bytes.end(), patchBytes.begin(), patchBytes.end());
//     }
//     return bytes;
// }

// LightPatch &DMXUniverse::operator[](int index)
// {
//     return m_LightPatches[index];
// };

// void DMXUniverse::printLightPatches()
// {
//     std::cout << "Num lights: " << m_LightPatches.size() << ", Total size: " << (m_Size / 8) << " Channels" << std::endl;
//     int prevOffset = 0;
//     unsigned int c = 0;
//     for (const auto &patch : m_LightPatches)
//     {
//         std::cout << "[" << c++ << "] ";
//         patch.printLight(prevOffset);
//         prevOffset += patch.size();
//     }
// }

// void DMXUniverse::printByteValue() const
// {
//     std::vector<uint8_t> bytes = byteValue();
//     std::cout << "Bytes: " << "\x1B[32m";
//     int i = 0;
//     for (; i < bytes.size(); i++)
//     {
//         if (i % 16 == 0)
//             std::cout << std::endl;
//         std::cout << padByte(bytes[i], 3) << "  ";
//     }

//     std::cout << "\x1B[0m";
//     while (i < 512)
//     {
//         if (i % 16 == 0)
//             std::cout << std::endl;
//         std::cout << padByte(0, 3) << "  ";

//         i++;
//     }
//     std::cout << std::endl;
// }
