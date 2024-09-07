#include <iostream>
#include <vector>
#include <stdio.h>

#define MAX_SIZE 20

struct Seg
{
    int start;
    int size;

    friend std::ostream &operator<<(std::ostream &os, const Seg &seg)
    {
        os << "Segment: [" << seg.start << ", " << seg.start + seg.size - 1 << "] Size: " << seg.size;
        return os;
    }
};

struct DMXUniverse
{
    std::vector<Seg> segments;

    bool addSegment(int size)
    {
        if (segments.empty())
        {
            if (size <= MAX_SIZE)
            {
                segments.push_back({0, size});
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
                segments.push_back({seg.start + seg.size, size});
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool addSegment(int start, int size)
    {
        if (segments.size() == 0)
        {
            if (start + size < MAX_SIZE)
            {
                segments.push_back({start, size});
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
                    segments.insert(segments.begin() + i + 1, {start, size});
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
                    segments.push_back({start, size});
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
        for (int i = 0; i < segments.size(); i++)
        {
            auto &curr = segments[i];
            std::cout << curr << std::endl;

            if (i < segments.size() - 1)
            {
                auto &next = segments[i + 1];
                if (curr.start + curr.size != next.start)
                {
                    std::cout << "Empty: [" << (curr.start + curr.size) << ", " << next.start - 1 << "]" << std::endl;
                    continue;
                }
            }
            else
            {
                if (curr.start + curr.size < MAX_SIZE)
                {
                    std::cout << "Empty: [" << (curr.start + curr.size) << ", " << MAX_SIZE - 1 << "]" << std::endl;
                    continue;
                }
            }
        }
    }
};

int main()
{
    DMXUniverse manager;
    manager.addSegment(0, 4);
    manager.addSegment(6, 2);
    manager.addSegment(12, 5);
    // manager.addSegment(8, 5);
    // manager.addSegment(5, 3);
    // manager.addSegment(2);
    // manager.addSegment(20, 1);
    manager.printSegments();
    return 0;
}