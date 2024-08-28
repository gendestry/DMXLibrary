#include <iostream>
#include "DMXUniverse.h"

int main()
{
    try
    {

        LightPatch patch("Eurolite RGBDS");
        patch.addPatchUnit({PatchUnit::COLOR_R, 255});
        patch.addPatchUnit({PatchUnit::COLOR_G});
        patch.addPatchUnit({PatchUnit::COLOR_B});
        patch.addPatchUnit({PatchUnit::INTENSITY});
        patch.addPatchUnit({PatchUnit::STROBE});

        patch[4].setValue(128);

        LightPatch patchRGB("RGB", {PatchUnit::COLOR_R, PatchUnit::COLOR_G, PatchUnit::COLOR_B, {PatchUnit::PAN16, 512}});

        DMXUniverse manager;
        manager.addLightPatch(patch);
        manager.addLightPatch(patch);
        manager.addLightPatch(patch);
        manager.addLightPatch(patchRGB);

        manager.printLightPatches();

        manager.printByteValue();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    // int size = 16;
    // unsigned int value = 512;
    // unsigned int s = size / 8;
    // std::cout << std::hex << value << std::endl;
    // // prit size
    // std::cout << "Size: " << s << std::endl;
    // for (int i = 0; i <= s; i++)
    // {
    //     int val = (value >> (i * 4)) & 0xF;
    //     printf("%d: ", i);
    //     printf("%x\n", val);

    //     // std::cout << std::hex << val << std::endl;
    // }

    return 0;
}
