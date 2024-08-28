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
