#include "utility.h"

int64_t mc_GetLE(void *src,int size)
{
    int64_t result;
    unsigned char *ptr;
    unsigned char *ptrEnd;
    int shift;

    ptr=(unsigned char*)src;
    ptrEnd=ptr+size;

    result=0;
    shift=0;
    while(ptr<ptrEnd)
    {
        result|=((int64_t)(*ptr))<<shift;
        shift+=8;
        ptr++;
    }

    return result;                                                              // Assuming all systems are little endian
}
