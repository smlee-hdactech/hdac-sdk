#ifndef UTILITY_H
#define UTILITY_H

#include "strcodeclib_global.h"
#include <cstdint>
#include "tools.h"

int64_t mc_GetLE(void *src,int size);

typedef struct mc_Buffer
{
    mc_Buffer()
    {
        Zero();
    }

    ~mc_Buffer()
    {
        Destroy();
    }

    mc_MapStringIndex      *m_lpIndex;
    unsigned char          *m_lpData;
    int                     m_AllocSize;
    int                     m_Size;
    int                     m_KeySize;
    int                     m_RowSize;
    int                     m_Count;
    uint32_t                m_Mode;

    void Zero();
    int Destroy();
    int Initialize(int KeySize,int TotalSize,uint32_t Mode);

    int Clear();
    int Realloc(int Rows);
    int Add(const void *lpKey,const void *lpValue);
    int Add(const void *lpKeyValue);
    int Seek(void *lpKey);
    unsigned char *GetRow(int RowID);
    int PutRow(int RowID,const void *lpKey,const void *lpValue);
    int GetCount();
    int SetCount(int count);

    int Sort();

    void CopyFrom(mc_Buffer *source);

} mc_Buffer;

void *mc_New(int Size);
void mc_Delete(void *ptr);
int64_t mc_GetVarInt(const unsigned char *buf,int max_size,int64_t default_value,int* shift);
int mc_PutVarInt(unsigned char *buf,int max_size,int64_t value);
void mc_PutLE(void *dest,void *src,int dest_size);
int64_t mc_GetLE(void *src,int size);

unsigned int mc_TimeNowAsUInt();

int mc_StringCompareCaseInsensitive(const char *str1,const char *str2,int len);

#endif // UTILITY_H
