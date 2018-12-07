#include "utilsfront.h"
#include "utilstrencodings.h"

using namespace std;

int32_t parseHexToInt32Le(const string& hexString)
{
    auto hexList = ParseHex(hexString);
    int32_t checksum = 0;
    for (int i = 0; i < hexList.size(); i++) {
        checksum |= ((int32_t)hexList[i]) << 8*i;
    }
    return checksum;
}
