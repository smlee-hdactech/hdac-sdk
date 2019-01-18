#include "utility.h"
#include "define.h"
#include <cstring>

#ifdef WIN32
#include <ctime>
#include <windows.h>
#endif // !WIN32

#ifndef WIN32
#include <sys/time.h>
#endif

#define MC_DCT_BUF_ALLOC_ITEMS          256
#define MC_DCT_LIST_ALLOC_MIN_SIZE      32768
#define MC_DCT_LIST_ALLOC_MAX_SIZE      268435456

int mc_AllocSize(int items,int chunk_size,int item_size)
{
    if(items<=0)
    {
        return 0;
    }
    return ((items-1)/chunk_size+1)*chunk_size*item_size;
}

void *mc_New(int Size)
{
    int TSize;
    int64_t *ptr;

    TSize=(Size-1)/sizeof(int64_t)+1;
    ptr=new int64_t[(size_t)TSize];

    if(ptr)
    {
        memset(ptr,0,(size_t)Size);
    }

    return ptr;
}

void mc_Delete(void *ptr)
{
    delete [] (int64_t*)ptr;
}

void mc_PutLE(void *dest,void *src,int dest_size)
{
    memcpy(dest,src,dest_size);                                                 // Assuming all systems are little endian
}

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

int mc_VarIntSize(unsigned char byte)
{
    if(byte<0xfd)return 0;
    if(byte==0xfd)return 2;
    if(byte==0xfe)return 4;
    return  8;
}

int64_t mc_GetVarInt(const unsigned char *buf,int max_size,int64_t default_value,int* shift)
{
    int size;
    if(max_size<=0)
    {
        return default_value;
    }

    size=mc_VarIntSize(buf[0]);

    if(max_size < size+1)
    {
        return default_value;
    }

    if(shift)
    {
        *shift=size+1;
    }

    if(size == 0)
    {
        return buf[0];
    }

    return mc_GetLE((void*)(buf+1),size);
}

int mc_PutVarInt(unsigned char *buf,int max_size,int64_t value)
{
    int varint_size,shift;

    if(max_size<=0)
    {
        return -1;
    }

    varint_size=1;
    shift=0;
    if(value>=0xfd)
    {
        shift=1;
        if(value>=0xffff)
        {
            if(value>=0xffffffff)
            {
                buf[0]=0xff;
                varint_size=8;
            }
            else
            {
                buf[0]=0xfe;
                varint_size=4;
            }
        }
        else
        {
            buf[0]=0xfd;
            varint_size=2;
        }
    }

    if(max_size < shift+varint_size)
    {
        return -1;
    }

    mc_PutLE(buf+shift,&value,varint_size);
    return shift+varint_size;
}

#ifdef WIN32

// epoch time?쇰줈 蹂?섑븷 ?곸닔
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

// for timezone
struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

// gettimeofday in windows
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		// system time??援ы븯湲?
		GetSystemTimeAsFileTime(&ft);

		// unsigned 64 bit濡?留뚮뱾湲?
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		// 100nano瑜?1micro濡?蹂?섑븯湲?
		tmpres /= 10;

		// epoch time?쇰줈 蹂?섑븯湲?
		tmpres -= DELTA_EPOCH_IN_MICROSECS;

		// sec? micorsec?쇰줈 留욎텛湲?
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (tmpres % 1000000UL);
	}

	// timezone 泥섎━
	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
#ifdef WIN32
		long sec;
		_get_timezone(&sec);
		tz->tz_minuteswest = sec / 60;
		int hour;
		_get_daylight(&hour);
		tz->tz_dsttime = hour;
#else
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
#endif
	}

	return 0;
}

#endif // !WIN32

unsigned int mc_TimeNowAsUInt()
{
    struct timeval time_now;

    gettimeofday(&time_now,NULL);

    return time_now.tv_sec;
}

void mc_Buffer::Zero()
{
    m_lpData=nullptr;
    m_lpIndex=nullptr;
    m_AllocSize=0;
    m_Size=0;
    m_KeySize=0;
    m_RowSize=0;
    m_Count=0;
    m_Mode=0;
}

int mc_Buffer::Destroy()
{
    if(m_lpIndex)
    {
        delete m_lpIndex;
    }
    if(m_lpData)
    {
        mc_Delete(m_lpData);
    }

    Zero();

    return MC_ERR_NOERROR;
}

int mc_Buffer::Initialize(int KeySize,int RowSize,uint32_t Mode)
{
    int err;

    err=MC_ERR_NOERROR;

    Destroy();

    m_Mode=Mode;
    m_KeySize=KeySize;
    m_RowSize=RowSize;

    if(m_Mode & MC_BUF_MODE_MAP)
    {
        m_lpIndex=new mc_MapStringIndex;
    }


    m_AllocSize=mc_AllocSize(1,MC_DCT_BUF_ALLOC_ITEMS,m_RowSize);

    m_lpData=(unsigned char*)mc_New(m_AllocSize);
    if(m_lpData==nullptr)
    {
        Zero();
        err=MC_ERR_ALLOCATION;
        return err;
    }

    return err;
}

int mc_Buffer::Clear()
{
    m_Size=0;
    m_Count=0;

    if(m_lpIndex)
    {
        m_lpIndex->Clear();
    }


    return MC_ERR_NOERROR;
}

int mc_Buffer::Realloc(int Rows)
{
    unsigned char *lpNewBuffer;
    int NewSize;
    int err;

    err=MC_ERR_NOERROR;

    if(m_Size+m_RowSize*Rows>m_AllocSize)
    {
        NewSize=mc_AllocSize(m_Count+Rows,MC_DCT_BUF_ALLOC_ITEMS,m_RowSize);
        lpNewBuffer=(unsigned char*)mc_New(NewSize);

        if(lpNewBuffer==NULL)
        {
            err=MC_ERR_ALLOCATION;
            return err;
        }

        memcpy(lpNewBuffer,m_lpData,m_AllocSize);
        mc_Delete(m_lpData);

        m_AllocSize=NewSize;
        m_lpData=lpNewBuffer;
    }

    return err;
}

int mc_Buffer::Add(const void *lpKey,const void *lpValue)
{
    int err;

    err=Realloc(1);
    if(err)
    {
        return err;
    }

    if(m_KeySize)
    {
        memcpy(m_lpData+m_Size,lpKey,m_KeySize);
    }

    if(m_KeySize<m_RowSize)
    {
        memcpy(m_lpData+m_Size+m_KeySize,lpValue,m_RowSize-m_KeySize);
    }

    m_Size+=m_RowSize;

    if(m_lpIndex)
    {
        m_lpIndex->Add((unsigned char*)lpKey,m_KeySize,m_Count);
    }

    m_Count++;

    return err;
}

int mc_Buffer::Add(const void *lpKeyValue)
{
    return Add(lpKeyValue,(unsigned char*)lpKeyValue+m_KeySize);
}

int mc_Buffer::PutRow(int RowID,const void *lpKey,const void *lpValue)
{
    unsigned char *ptr;

    if(RowID>=m_Count)
    {
        return MC_ERR_INTERNAL_ERROR;
    }

    ptr=m_lpData+m_RowSize*RowID;

    if(m_KeySize)
    {
        memcpy(ptr,lpKey,m_KeySize);
    }

    if(m_KeySize<m_RowSize)
    {
        memcpy(ptr+m_KeySize,lpValue,m_RowSize-m_KeySize);
    }

    if(m_lpIndex)
    {
        m_lpIndex->Add((unsigned char*)lpKey,m_KeySize,RowID);
    }

    return MC_ERR_NOERROR;
}


int mc_Buffer::Seek(void *lpKey)
{
    unsigned char *ptr;
    int row;

    if(m_lpIndex)
    {
        row=m_lpIndex->Get((unsigned char*)lpKey,m_KeySize);
        if(row >= 0)
        {
            ptr=GetRow(row);
            if(memcmp(ptr,lpKey,m_KeySize)==0)
            {
                return row;
            }
        }
        return -1;
    }

    ptr=m_lpData;
    row=0;

    while(row<m_Count)
    {
        if(memcmp(ptr,lpKey,m_KeySize)==0)
        {
            return row;
        }
        ptr+=m_RowSize;
        row++;
    }

    return -1;
}


unsigned char *mc_Buffer::GetRow(int RowID)
{
    return m_lpData+m_RowSize*RowID;
}

int mc_Buffer::GetCount()
{
    return m_Count;
}

int mc_Buffer::SetCount(int count)
{
    int i;
    int err=MC_ERR_NOERROR;

    if(m_Count>count)
    {
        if(m_lpIndex)
        {
            m_lpIndex->Clear();
            m_Count=0;
            m_Size=0;
            for(i=0;i<count;i++)
            {
                Add(GetRow(i),GetRow(i)+m_KeySize);
            }
        }
        else
        {
            m_Count=count;
        }
    }

    if(m_Count<count)
    {
        err=Realloc(count-m_Count);
    }

    m_Count=count;
    m_Size=count*m_RowSize;

    return err;
}

void mc_Buffer::CopyFrom(mc_Buffer *source)
{
    Clear();
    int i;
    unsigned char *ptr;

    for(i=0;i<source->GetCount();i++)
    {
        ptr=source->GetRow(i);
        Add(ptr,ptr+m_KeySize);
    }
}

int mc_Buffer::Sort()
{
    if(m_lpIndex)
    {
        return MC_ERR_NOT_SUPPORTED;
    }

    if(m_Count <= 1)
    {
        return MC_ERR_NOERROR;
    }

    int i,j,t;
    int err;

    err=Realloc(1);
    if(err)
    {
        return err;
    }

    t=m_AllocSize/m_RowSize-1;

    for(i=0;i<m_Count-1;i++)                                                    // This is never used, so we can consider entire function as stub
    {
        for(j=i;j>=0;j--)
        {
            if(memcmp(GetRow(j),GetRow(j+1),m_RowSize) > 0)
            {
                memcpy(GetRow(t),GetRow(j+1),m_RowSize);
                memcpy(GetRow(j+1),GetRow(j),m_RowSize);
                memcpy(GetRow(j),GetRow(t),m_RowSize);
            }
        }
    }

    return MC_ERR_NOERROR;
}

int mc_StringCompareCaseInsensitive(const char *str1,const char *str2, int len)
{
    int i,res;

    res=0;

    for(i=0;i<len;i++)
    if(res==0)
    {
        if(str1[i]!=str2[i])
        {
            res=1;
            if(str1[i]>0x60&&str1[i]<0x7b)
            if(str1[i]-0x20==str2[i])
            res=0;
            if(res)
            {
                if(str2[i]>0x60&&str2[i]<0x7b)
                if(str2[i]-0x20==str1[i])
                res=0;
            }
        }
    }

    return res;
}
