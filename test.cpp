#include <iostream>
#include <vector>
#include <stdio.h>
#include <string>
#include <unordered_map>

enum Type
{
    R,
    G,
    B
};

struct Light
{
    int value;
};

std::unordered_map<Type, std::vector<int>> map;
std::unordered_map<std::string, std::vector<Light *>> lightMap;

int main()
{
    // map[R].push_back(1);
    // map[R].push_back(2);
    // map[R].push_back(4);
    // map[G].push_back(3);

    // for (const auto &pair : map)
    // {
    //     std::cout << "Type: " << pair.first << std::endl;
    //     for (const auto &value : pair.second)
    //     {
    //         std::cout << "Value: " << value << std::endl;
    //     }
    // }

    lightMap["Led PX"].push_back(new Light{1});
    lightMap["Led PX"].push_back(new Light{2});
    lightMap["Led PX"].push_back(new Light{3});
    lightMap["Led PXASDASD"].push_back(new Light{4});

    for (const auto &pair : lightMap)
    {
        std::cout << "Name: " << pair.first << std::endl;
        for (const auto &light : pair.second)
        {
            std::cout << "Value: " << light->value << std::endl;
        }
    }

    return 0;
}