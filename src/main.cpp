// #include <iostream>
// #include "Neki.h"

#include "DMXUniverse.h"

// struct Store
// {

//     struct StoreUnit
//     {
//         int size;
//     };

//     struct Segment
//     {
//         unsigned int start;
//         StoreUnit unit;

//         Segment(unsigned int start, StoreUnit unit) : start(start), unit(unit) {}

//         unsigned int getStartAddress()
//         {
//             return start;
//         }

//         unsigned int getSize()
//         {
//             return unit.size;
//         }

//         unsigned int getEndAddress()
//         {
//             return start + unit.size - 1;
//         }

//         unsigned int getNextAddress()
//         {
//             return start + unit.size;
//         }
//     };

//     std::list<Segment> segments;

//     void addSegment(StoreUnit storeUnit)
//     {
//         // if empty, push to start
//         if (segments.empty())
//         {
//             if (storeUnit.size > MAX_SIZE)
//             {
//                 throw std::runtime_error("[1]Segment exceeds max size");
//             }
//             segments.push_back({0, storeUnit});
//         }
//         else
//         {
//             auto &lastSegment = segments.back();
//             if (lastSegment.getNextAddress() + storeUnit.size > MAX_SIZE)
//             {
//                 throw std::runtime_error("[2]Segment exceeds max size");
//             }
//             segments.push_back({lastSegment.getNextAddress(), storeUnit});
//         }
//     }

//     void addSegment(StoreUnit storeUnit, unsigned int start)
//     {
//         std::cout << "Adding segment: [" << start << ", " << start + storeUnit.size - 1 << "] Size: " << storeUnit.size << std::endl;
//         // print debug
//         if (segments.empty())
//         {
//             if (storeUnit.size > MAX_SIZE)
//             {
//                 throw std::runtime_error("[1]Segment exceeds max size");
//             }
//             segments.push_back({start, storeUnit});
//             return;
//         }

//         auto it = segments.begin();
//         auto oldIt = it;

//         for (; it != segments.end(); it++)
//         {
//             auto &segment = *it;
//             unsigned int segStart = segment.getStartAddress();
//             unsigned int segEnd = segment.getEndAddress();

//             // if start is higher than segment, skip

//             // if within segment
//             if (start >= segStart && start <= segEnd)
//             {
//                 throw std::runtime_error("[4]Segment overlaps with existing segment");
//             }

//             oldIt = it;

//             if (start > segEnd)
//             {
//                 continue;
//             }
//             else
//             {
//                 it++;
//                 break;
//             }
//         }

//         // print values in if
//         std::cout << "   Segment: [" << oldIt->getStartAddress() << ", " << oldIt->getEndAddress() << "] Size: " << oldIt->getSize() << std::endl;
//         std::cout << "   Segment: [" << it->getStartAddress() << ", " << it->getEndAddress() << "] Size: " << it->getSize() << std::endl;

//         if (it != segments.end() && oldIt->getEndAddress() + storeUnit.size >= it->getStartAddress())
//         {
//             throw std::runtime_error("[5]Segment overlaps with existing segment");
//         }
//         else if (storeUnit.size + start > MAX_SIZE)
//         {
//             throw std::runtime_error("[6]Segment exceeds max size");
//         }
//         segments.insert(oldIt, {start, storeUnit});

//         // {

//         //     oldIt = it;
//         //     it++;
//         //     break;
//         // }

//         // if (it != segments.end() && oldIt->getEndAddress() + storeUnit.size >= it->getStartAddress())
//         // {
//         //     throw std::runtime_error("Trying to insert a segment withing a segment");
//         // }

//         // else if (storeUnit.size + start > MAX_SIZE)
//         // {
//         //     throw std::runtime_error("Segment does not fit at the end");
//         // }

//         // std::cout << "   Segment: [" << start << ", " << start + storeUnit.size - 1 << "] Size: " << storeUnit.size << std::endl;
//         // // print old it

//         // segments.insert(oldIt, {start, storeUnit});
//     }

//     void printList()
//     {
//         unsigned int prev = 0;

//         for (auto &segment : segments)
//         {
//             unsigned int segstart = segment.getStartAddress();
//             unsigned int segend = segment.getNextAddress();

//             if (segstart != prev)
//             {
//                 std::cout << "[E]Segment: [" << prev << ", " << segstart - 1 << "] Size: " << segstart - prev << std::endl;
//             }
//             std::cout << "[D]Segment: [" << segstart << ", " << segment.getEndAddress() << "] Size: " << segment.getSize() << std::endl;
//             prev = segment.getNextAddress();
//         }

//         if (prev < MAX_SIZE)
//         {
//             std::cout << "[E]Segment: [" << prev << ", " << MAX_SIZE - 1 << "] Size: " << MAX_SIZE - prev << std::endl;
//         }

//         std::cout << "----------------" << std::endl;
//     }
// };

int main()
{
    DMXUniverse manager;
    // LightPatch *patch2 = new LightPatch("Eurolite RGBDS", {PatchUnit::COLOR_R, PatchUnit::COLOR_G, PatchUnit::COLOR_B});
    LightPatch eurolite("Eurolite RGBDS", {PatchUnit::COLOR_R, PatchUnit::COLOR_G, PatchUnit::COLOR_B, PatchUnit::INTENSITY, PatchUnit::STROBE});
    LightPatch ledPx3("Led PX", {PatchUnit::COLOR_R, PatchUnit::COLOR_G, PatchUnit::COLOR_B, PatchUnit::COLOR_R, PatchUnit::COLOR_G, PatchUnit::COLOR_B, PatchUnit::COLOR_R, PatchUnit::COLOR_G, PatchUnit::COLOR_B});
    // manager.addSegment(eurolite);
    manager.addSegment(ledPx3);
    manager.addSegment(ledPx3);
    manager.printSegments();

    auto v = manager[1].valuesForType(PatchUnit::COLOR_R);

    int i = 1;
    for (unsigned int *val : v)
    {
        *val = i++;
    }

    std::vector<LightPatch *> vec = manager["Led PX"];
    std::cout << "Led PX size: " << vec.size() << std::endl;

    for (int i = 0; i < vec.size(); i++)
    {
        std::cout << *vec[i] << std::endl;
        std::cout << i << " " << vec[i]->getName() << std::endl;
    }
    // for (LightPatch *patch : vec)
    // {
    //     std::cout << patch->getName() << std::endl;
    // }
    // manager.printByteValue();

    // Store store;
    // store.addSegment({4}, 0);
    // store.addSegment({2}, 5);
    // store.addSegment({3}, 10);
    // store.printList();
    // store.addSegment({3}, 7);
    // store.printList();

    // std::list<int> l = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    // auto it = l.begin();
    // auto oldIt = it;

    // // print current and previous value for loop
    // for (; it != l.end(); it++)
    // {
    //     std::cout << "Current: " << *it << ", Previous: " << *oldIt << std::endl;
    //     oldIt = it;
    // }

    // store.addSegment({5}, 15);
    // store.printList();

    // store.addSegment({10});
    // store.printList();

    // store.addSegment({2}, 10);

    // // store.addSegment({15});
    // store.printList();
    // try
    // {

    //     LightPatch patch("Eurolite RGBDS");
    //     patch.addPatchUnit({PatchUnit::COLOR_R, 255});
    //     patch.addPatchUnit({PatchUnit::COLOR_G});
    //     patch.addPatchUnit({PatchUnit::COLOR_B});
    //     patch.addPatchUnit({PatchUnit::INTENSITY});
    //     patch.addPatchUnit({PatchUnit::STROBE});

    //     patch[4].setValue(128);

    //     LightPatch patchRGB("RGB", {PatchUnit::COLOR_R, PatchUnit::COLOR_G, PatchUnit::COLOR_B, {PatchUnit::PAN16, 512}});

    //     DMXUniverse manager;
    //     manager.addLightPatch(patch);
    //     manager.addLightPatch(patch);
    //     manager.addLightPatch(patch);
    //     manager.addLightPatch(patchRGB);

    //     manager.printLightPatches();

    //     manager.printByteValue();
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << e.what() << '\n';
    // }

    return 0;
}
