﻿#ifndef STRCODECLIB_H
#define STRCODECLIB_H

#include <string>
#include <vector>

std::vector<unsigned char> ParseHex(const char* psz);
std::vector<unsigned char> ParseHex(const char* psz,bool &fIsHex);
std::vector<unsigned char> ParseHex(const std::string& str);

signed char HexDigit(char c);
bool IsHex(const std::string& str);

template<typename T>
std::string HexStr(const T itbegin, const T itend, bool fSpaces=false)
{
    std::string rv;
    static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    rv.reserve((itend-itbegin)*3);
    for(T it = itbegin; it < itend; ++it)
    {
        unsigned char val = (unsigned char)(*it);
        if(fSpaces && it != itbegin)
            rv.push_back(' ');
        rv.push_back(hexmap[val>>4]);
        rv.push_back(hexmap[val&15]);
    }

    return rv;
}

template<typename T>
inline std::string HexStr(const T& vch, bool fSpaces=false)
{
    return HexStr(vch.begin(), vch.end(), fSpaces);
}

int32_t parseHexToInt32Le(const std::string& hexString);

#endif // STRCODECLIB_H
