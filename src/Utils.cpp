#include "Utils.h"

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
