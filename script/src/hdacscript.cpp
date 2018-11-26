#include "hdacscript.h"
#include <utils/define.h>
#include <utils/utility.h>

#define MC_DCT_SCRIPT_OP_PUSHDATA1       0x4c
#define MC_DCT_SCRIPT_OP_PUSHDATA2       0x4d
#define MC_DCT_SCRIPT_OP_PUSHDATA4       0x4e
#define MC_DCT_SCRIPT_OP_16              0x60
#define MC_DCT_SCRIPT_OP_RETURN          0x6a
#define MC_DCT_SCRIPT_OP_DROP            0x75

int mc_GetPushDataElement(unsigned char *src,int size,int *op_drop_offset,int *op_drop_size)
{
    unsigned char opcode;
    unsigned char *ptr;
    unsigned char *ptrEnd;
    int nSize;

    ptr=(unsigned char*)src;
    ptrEnd=ptr+size;

    if(ptr >= ptrEnd)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    nSize=0;
    opcode=*ptr++;

    if (opcode <= MC_DCT_SCRIPT_OP_PUSHDATA4)
    {
        nSize = 0;
        if (opcode < MC_DCT_SCRIPT_OP_PUSHDATA1)
        {
            nSize = (int)opcode;
        }
        else if (opcode == MC_DCT_SCRIPT_OP_PUSHDATA1)
        {
            if (ptrEnd - ptr < 1)
            {
                return MC_ERR_WRONG_SCRIPT;
            }
            nSize = mc_GetLE(ptr,1);
            ptr++;
        }
        else if (opcode == MC_DCT_SCRIPT_OP_PUSHDATA2)
        {
            if (ptrEnd - ptr < 2)
            {
                return MC_ERR_WRONG_SCRIPT;
            }
            nSize = mc_GetLE(ptr,2);
            ptr+=2;
        }
        else if (opcode == MC_DCT_SCRIPT_OP_PUSHDATA4)
        {
            if (ptrEnd - ptr < 4)
            {
                return MC_ERR_WRONG_SCRIPT;
            }
            nSize = mc_GetLE(ptr,4);
            ptr+=4;
        }

        if(ptrEnd<ptr+nSize)
        {
            return MC_ERR_WRONG_SCRIPT;
        }
    }
    else
    {
        if(opcode > MC_DCT_SCRIPT_OP_16)
        {
            return MC_ERR_WRONG_SCRIPT;
        }
    }

    *op_drop_offset=ptr-src;
    *op_drop_size=nSize;

    return MC_ERR_NOERROR;
}

const unsigned char *mc_ParseOpDropOpReturnScript(const unsigned char *src,int size,int *op_drop_offset,int *op_drop_size,int op_drop_count,int *op_return_offset,int *op_return_size)
{
    unsigned char *ptr;
    unsigned char *ptrEnd;
    int d;
    int elem_offset, elem_size;

    ptr=(unsigned char*)src;
    ptrEnd=ptr+size;

    d=0;
    while( (d<op_drop_count) && (ptr<ptrEnd) && (*ptr != MC_DCT_SCRIPT_OP_RETURN) )
    {
        op_drop_offset[d]=0;
        op_drop_size[d]=-1;
        if(mc_GetPushDataElement(ptr,ptrEnd-ptr,&elem_offset,&elem_size) != MC_ERR_WRONG_SCRIPT)
        {
            elem_offset+=ptr-src;
            ptr=(unsigned char*)src+elem_offset+elem_size;
            if(ptr < ptrEnd)
            {
                if(*ptr == MC_DCT_SCRIPT_OP_DROP)
                {
                    op_drop_offset[d]=elem_offset;
                    op_drop_size[d]=elem_size;
                    d++;
                    ptr++;
                }
            }
        }
        else
        {
            ptr++;
        }
    }

    while( (ptr<ptrEnd) && (*ptr != MC_DCT_SCRIPT_OP_RETURN) )
    {
        if(mc_GetPushDataElement(ptr,ptrEnd-ptr,&elem_offset,&elem_size) == MC_ERR_WRONG_SCRIPT)
        {
            ptr++;
        }
        else
        {
            ptr+=elem_offset+elem_size;
        }
    }

    if(ptr >= ptrEnd)
    {
        return nullptr;
    }

    ptr++;
    if(mc_GetPushDataElement(ptr,ptrEnd-ptr,op_return_offset,op_return_size) == MC_ERR_WRONG_SCRIPT)
    {
        *op_return_offset=ptr-src;
        *op_return_size=0;
    }
    else
    {
        *op_return_offset+=ptr-src;
    }

    return src;
}
