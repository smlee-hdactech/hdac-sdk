﻿#include "hdacscript.h"
#include <utils/define.h>
#include <utils/utility.h>
#include <cstring>
#include <entities/asset.h>

#define MC_DCT_SCRIPT_OP_PUSHDATA1       0x4c
#define MC_DCT_SCRIPT_OP_PUSHDATA2       0x4d
#define MC_DCT_SCRIPT_OP_PUSHDATA4       0x4e
#define MC_DCT_SCRIPT_OP_16              0x60
#define MC_DCT_SCRIPT_OP_RETURN          0x6a
#define MC_DCT_SCRIPT_OP_DROP            0x75

#define MC_DCT_SCRIPT_FREE_DATA_IDENTIFIER     "SPK"
#define MC_DCT_SCRIPT_HDAC_IDENTIFIER    "spk"
#define MC_DCT_SCRIPT_IDENTIFIER_LEN 3

#define MC_DCT_SCRIPT_HDAC_ENTITY_PREFIX 'e'
#define MC_DCT_SCRIPT_HDAC_CASHED_SCRIPT_PREFIX 'i'
#define MC_DCT_SCRIPT_HDAC_KEY_PREFIX 'k'
#define MC_DCT_SCRIPT_HDAC_APPROVE_PREFIX 'a'
#define MC_DCT_SCRIPT_HDAC_NEW_ENTITY_PREFIX 'n'
#define MC_DCT_SCRIPT_HDAC_UPDATE_ENTITY_PREFIX 'u'
#define MC_DCT_SCRIPT_HDAC_PERMISSIONS_PREFIX 'p'
#define MC_DCT_SCRIPT_HDAC_BLOCK_SIGNATURE_PREFIX 'b'
#define MC_DCT_SCRIPT_HDAC_ASSET_GENESIS_PREFIX 'g'
#define MC_DCT_SCRIPT_HDAC_ASSET_QUANTITY_PREFIX 'q'
#define MC_DCT_SCRIPT_HDAC_ASSET_DETAILS_PREFIX 'a'
#define MC_DCT_SCRIPT_HDAC_ASSET_FOLLOWON_PREFIX 'o'
#define MC_DCT_SCRIPT_HDAC_GENERAL_DETAILS_PREFIX 'c'

#define MC_DCT_SCRIPT_ALLOC_BUFFER_CHUNK 4096
#define MC_DCT_SCRIPT_ALLOC_INDEX_CHUNK    16

#define MC_DCT_SCRIPT_TYPE_REGULAR                         0x00
#define MC_DCT_SCRIPT_TYPE_OP_RETURN                       0x01
#define MC_DCT_SCRIPT_TYPE_OP_DROP                         0x02
#define MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN                 0x04

int mc_Script::Zero()
{
    m_Size=0;
    m_NumElements=0;
    m_CurrentElement=-1;
    m_lpData=NULL;
    m_lpCoord=NULL;
    m_AllocElements=0;
    m_AllocSize=0;
    m_ScriptType=MC_DCT_SCRIPT_TYPE_REGULAR;

    return MC_ERR_NOERROR;
}

int mc_Script::Destroy()
{
    if(m_lpData)
    {
        mc_Delete(m_lpData);
    }
    if(m_lpCoord)
    {
        mc_Delete(m_lpCoord);
    }

    return Zero();
}

int mc_Script::Resize(size_t bytes,int elements)
{
    int NewSize;
    unsigned char *lpNewBuffer;
    int *lpNewCoord;

    NewSize=m_AllocSize;
    while(m_Size+(int)bytes > NewSize)
    {
        NewSize+=MC_DCT_SCRIPT_ALLOC_BUFFER_CHUNK;
    }

    if(NewSize > m_AllocSize)
    {
        lpNewBuffer=(unsigned char *)mc_New(NewSize);
        if(lpNewBuffer == NULL)
        {
            return MC_ERR_ALLOCATION;
        }
        if(m_lpData)
        {
            if(m_Size)
            {
                memcpy(lpNewBuffer,m_lpData,m_Size);
            }
            mc_Delete(m_lpData);
        }
        m_lpData=lpNewBuffer;
        m_AllocSize=NewSize;
    }

    NewSize=m_AllocElements;
    while(m_NumElements+elements > NewSize)
    {
        NewSize+=MC_DCT_SCRIPT_ALLOC_INDEX_CHUNK;
    }

    if(NewSize > m_AllocElements)
    {
        lpNewCoord=(int *)mc_New(NewSize*2*sizeof(int));
        if(lpNewCoord == NULL)
        {
            return MC_ERR_ALLOCATION;
        }
        if(m_lpCoord)
        {
            if(m_NumElements)
            {
                memcpy(lpNewCoord,m_lpCoord,m_NumElements*2*sizeof(int));
            }
            mc_Delete(m_lpCoord);
        }
        m_lpCoord=lpNewCoord;
        m_AllocElements=NewSize;
    }

    return MC_ERR_NOERROR;
}

int mc_Script::Clear()
{
    m_Size=0;
    m_NumElements=0;
    m_CurrentElement=-1;
    m_ScriptType=MC_DCT_SCRIPT_TYPE_REGULAR;

    return MC_ERR_NOERROR;
}

int mc_Script::GetNumElements()
{
    return m_NumElements;
}

int mc_Script::AddElement()
{
    int err;

    err=Resize(0,1);
    if(err)
    {
        return err;
    }

    m_NumElements++;
    m_CurrentElement++;
    m_lpCoord[2*m_CurrentElement + 0]=m_Size;
    m_lpCoord[2*m_CurrentElement + 1]=0;

    return MC_ERR_NOERROR;
}

int mc_Script::GetElement()
{
    return m_CurrentElement;
}

int mc_Script::SetElement(int element)
{
    if(element >= m_NumElements)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    m_CurrentElement=element;
    return MC_ERR_NOERROR;
}

const unsigned char*  mc_Script::GetData(int element, size_t* bytes)
{
    if(element >= m_NumElements)
    {
        return NULL;
    }

    m_CurrentElement=element;
    if(bytes)
    {
        *bytes=m_lpCoord[2*m_CurrentElement + 1];
    }

    return m_lpData+m_lpCoord[2*m_CurrentElement + 0];
}

int mc_Script::SetData(const unsigned char* src, const size_t bytes)
{
    int err;

    err=Resize(bytes,0);
    if(err)
    {
        return err;
    }

    if(bytes)
    {
        memcpy(m_lpData+m_lpCoord[2*m_CurrentElement + 0]+m_lpCoord[2*m_CurrentElement + 1],src,bytes);
        m_lpCoord[2*m_CurrentElement + 1]+=bytes;
        m_Size+=bytes;
    }

    return MC_ERR_NOERROR;
}

int mc_Script::SetSpecialParamValue(unsigned char param, const unsigned char* param_value, const size_t param_value_size)
{
    unsigned char buf[16];
    int size,err;

    err=MC_ERR_NOERROR;

    buf[0]=0x00;
    buf[1]=param;
    err=SetData(buf,2);
    if(err)
    {
        return err;
    }

    size=mc_PutVarInt(buf,16,param_value_size);
    err=SetData(buf,size);
    if(err)
    {
        return err;
    }

    err=SetData(param_value,param_value_size);
    if(err)
    {
        return err;
    }

    return MC_ERR_NOERROR;
}

int mc_Script::SetParamValue(const char *param_name,const size_t param_name_size,const unsigned char* param_value,const size_t param_value_size)
{
    unsigned char buf[16];
    int size,err;

    err=MC_ERR_NOERROR;

    err=SetData((unsigned char*)param_name,param_name_size);
    if(err)
    {
        return err;
    }

    buf[0]=0x00;
    err=SetData(buf,1);
    if(err)
    {
        return err;
    }

    size=mc_PutVarInt(buf,16,param_value_size);
    err=SetData(buf,size);
    if(err)
    {
        return err;
    }

    err=SetData(param_value,param_value_size);
    if(err)
    {
        return err;
    }

    return MC_ERR_NOERROR;
}

size_t mc_Script::GetParamValue(const unsigned char *ptr,size_t total,size_t offset,size_t* param_value_start,size_t *bytes)
{
    int shift,name_size,value_size,size;

    *param_value_start=0;

    if(offset>=total)
    {
        return total;
    }

    name_size=strlen((char*)ptr+offset)+1;
    if(offset+name_size >= total)
    {
        return total;
    }

    value_size=mc_GetVarInt(ptr+offset+name_size,total-offset-name_size,-1,&shift);
    if(value_size<0)
    {
        return total;
    }

    size=name_size+shift+value_size;
    if(offset+size>total)
    {
        return total;
    }

    *bytes=value_size;
    *param_value_start=offset+name_size+shift;

    return offset+size;
}


int mc_Script::IsOpReturnScript()
{
    if(m_ScriptType & MC_DCT_SCRIPT_TYPE_OP_RETURN)
    {
        return 1;
    }

    return 0;
}

int mc_Script::IsDirtyOpReturnScript()
{
    if(m_ScriptType & MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN)
    {
        return 1;
    }

    return 0;
}

int mc_Script::SetScript(const unsigned char* src,const size_t bytes,int type)
{
    unsigned char opcode;
    unsigned char *ptr;
    unsigned char *ptrEnd;
    unsigned char *ptrPrev;
    int nSize,lastSize;
    int is_op_return,take_it,is_redeem_script;

    Clear();

    if(bytes == 0)
    {
        return MC_ERR_NOERROR;
    }

    is_op_return=0;
    ptr=(unsigned char*)src;
    ptrEnd=ptr+bytes;
    ptrPrev=ptr;
    lastSize=-1;

    while(ptr<ptrEnd)
    {
        take_it=0;
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

        if((type == MC_SCR_TYPE_SCRIPTPUBKEY) && (is_op_return == 0))
        {
            if(opcode == MC_DCT_SCRIPT_OP_RETURN)
            {
                is_op_return=1;
                m_ScriptType |= MC_DCT_SCRIPT_TYPE_OP_RETURN;
                if(lastSize >= 0)                                               // Push data before OP_RETURN
                {
                    m_ScriptType |= MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN;
                }
            }
            else
            {
                if(opcode == MC_DCT_SCRIPT_OP_DROP)
                {
                    m_ScriptType |= MC_DCT_SCRIPT_TYPE_OP_DROP;
                    if(lastSize >= 0)                                           // OP_DROP after push data
                    {
                        take_it=1;
                    }
                    else                                                        // Duplicate OP_DROP or OP_DROP without push data
                    {
                        m_ScriptType |= MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN;
                    }
                }
                else
                {
                    if(lastSize >= 0)                                           // Duplicate push data
                    {
                        m_ScriptType |= MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN;
                    }
                }
            }
            if(take_it)
            {
                AddElement();
                SetData(ptrPrev,lastSize);
            }
        }
        else
        {
            is_redeem_script=0;
            if( (type == MC_SCR_TYPE_SCRIPTSIG) && (nSize>1) )
            {
                if( (*ptr>0x50) && (*ptr<=0x60))
                {
                    if( m_NumElements > (*ptr-0x50) )                           // OP_0 with at least required number of signatures
                    {
                        is_redeem_script=1;
                    }
                }
            }

            if(is_redeem_script)
            {
                nSize=0;
            }
            else
            {
                AddElement();
                if(nSize)
                {
                    SetData(ptr,nSize);
                }
                else
                {
                    if(is_op_return)                                            // Empty OP_RETURN
                    {
                        SetData(ptr,0);
                    }
                    else
                    {
                        SetData(ptr-1,1);
                    }
                }
                if(is_op_return)                                                // Everything after element after OP_RETURN is ignored
                {
                    ptr=ptrEnd;
                }
                is_op_return=0;                                                 // Reset flag to avoid adding two empty elements
            }
        }

        ptrPrev=ptr;
        lastSize=nSize;
        if(is_op_return == 0)
        {
            if(lastSize == 0)
            {
                if(opcode > MC_DCT_SCRIPT_OP_16)                            // Not push data
                {
                    lastSize=-1;
                }
            }
        }

        if(lastSize<0)
        {
            if(opcode != MC_DCT_SCRIPT_OP_RETURN)
            {
                if(opcode != MC_DCT_SCRIPT_OP_DROP)
                {
                    m_ScriptType |= MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN;
                }
            }
        }


        ptr+=nSize;
    }

    if(is_op_return)                                                            // Empty OP_RETURN
    {
        AddElement();
        SetData(ptr,0);
    }

    if(m_ScriptType & MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN)
    {
        if( (m_ScriptType & MC_DCT_SCRIPT_TYPE_OP_RETURN ) == 0)
        {
            m_ScriptType -= MC_DCT_SCRIPT_TYPE_DIRTY_OP_RETURN;
        }
    }

    return MC_ERR_NOERROR;
}


int mc_Script::GetPermission(uint32_t *type,uint32_t *from,uint32_t *to,uint32_t *timestamp)
{
    unsigned char *ptr;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] != MC_DCT_SCRIPT_IDENTIFIER_LEN+1+16)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_PERMISSIONS_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
    *type=     (uint32_t)mc_GetLE(ptr+ 0,4);
    *from=     (uint32_t)mc_GetLE(ptr+ 4,4);
    *to       =(uint32_t)mc_GetLE(ptr+ 8,4);
    *timestamp=(uint32_t)mc_GetLE(ptr+12,4);

    return MC_ERR_NOERROR;
}

int mc_Script::SetPermission(uint32_t type,uint32_t from,uint32_t to,uint32_t timestamp)
{
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+16];
    unsigned char *ptr;
    int err;

    err=AddElement();
    if(err)
    {
        return err;
    }

    ptr=buf;
    memcpy(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_PERMISSIONS_PREFIX;

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
    mc_PutLE(ptr+ 0,&type,4);
    mc_PutLE(ptr+ 4,&from,4);
    mc_PutLE(ptr+ 8,&to,4);
    mc_PutLE(ptr+12,&timestamp,4);

    return SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1+16);
}


int mc_Script::GetBlockSignature(unsigned char* sig,int *sig_size,uint32_t* hash_type,unsigned char* key,int *key_size)
{
    unsigned char *ptr;
    int sig_len,key_len;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1+3)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];


    if(memcmp(ptr,MC_DCT_SCRIPT_FREE_DATA_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_BLOCK_SIGNATURE_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    sig_len=mc_GetLE(ptr,1);
    ptr++;

    if(sig_len>*sig_size)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1+3+sig_len)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    memcpy(sig,ptr,sig_len);
    *sig_size=sig_len;
    ptr+=sig_len;


    *hash_type=(uint32_t)mc_GetLE(ptr,1);
    ptr++;

    key_len=mc_GetLE(ptr,1);
    ptr++;

    if(key_len>*key_size)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(m_lpCoord[m_CurrentElement*2+1] != MC_DCT_SCRIPT_IDENTIFIER_LEN+1+3+sig_len+key_len)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    memcpy(key,ptr,key_len);
    *key_size=key_len;
    ptr+=key_len;

    return MC_ERR_NOERROR;
}

int mc_Script::SetBlockSignature(const unsigned char* sig,int sig_size,uint32_t hash_type,const unsigned char* key,int key_size)
{
    int err;
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1];

    if((sig_size>0xff) || (key_size>0xff))
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    err=AddElement();
    if(err)
    {
        return err;
    }

    memcpy(buf,MC_DCT_SCRIPT_FREE_DATA_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_BLOCK_SIGNATURE_PREFIX;

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1);
    if(err)
    {
        return err;
    }

    mc_PutLE(buf,&sig_size,1);
    err=SetData(buf,1);
    if(err)
    {
        return err;
    }

    err=SetData(sig,sig_size);
    if(err)
    {
        return err;
    }

    mc_PutLE(buf,&hash_type,1);
    err=SetData(buf,1);
    if(err)
    {
        return err;
    }

    mc_PutLE(buf,&key_size,1);
    err=SetData(buf,1);
    if(err)
    {
        return err;
    }

    err=SetData(key,key_size);
    if(err)
    {
        return err;
    }

    return MC_ERR_NOERROR;
}

int mc_Script::GetAssetGenesis(int64_t *quantity)
{
    unsigned char *ptr;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] != MC_DCT_SCRIPT_IDENTIFIER_LEN+1+8)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_ASSET_GENESIS_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
    *quantity=(uint64_t)mc_GetLE(ptr+ 0,8);

    if(*quantity < 0)
    {
        return MC_ERR_NOT_ALLOWED;
    }

    return MC_ERR_NOERROR;
}

int mc_Script::SetAssetGenesis(int64_t quantity)
{
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+8];
    unsigned char *ptr;
    int err;

    if(quantity < 0)
    {
        return MC_ERR_NOT_ALLOWED;
    }

    err=AddElement();
    if(err)
    {
        return err;
    }

    ptr=buf;
    memcpy(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_ASSET_GENESIS_PREFIX;

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
    mc_PutLE(ptr+ 0,&quantity,8);

    return SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1+8);
}

int mc_Script::GetAssetDetails(char* name,int* multiple,unsigned char* script,int *script_size)
{
    unsigned char *ptr;
    unsigned char *ptrEnd;
    unsigned char *ptrStart;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(GetGeneralDetails(script,script_size) == MC_ERR_NOERROR)
    {
        name[0]=0;
        *multiple=1;
        return MC_ERR_NOERROR;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1 + 4 + 1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(m_lpCoord[m_CurrentElement*2+1] > MC_DCT_SCRIPT_IDENTIFIER_LEN+1+MC_ENT_MAX_SCRIPT_SIZE + 4 + MC_ENT_MAX_NAME_SIZE + 1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];
    ptrEnd=ptr+m_lpCoord[m_CurrentElement*2+1];

    if(memcmp(ptr,MC_DCT_SCRIPT_FREE_DATA_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_ASSET_DETAILS_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    *multiple=(int)mc_GetLE(ptr+ 0,4);
    ptr+=4;

    ptrStart=ptr;
    while(*ptr)
    {
        if(ptr >= ptrStart+MC_ENT_MAX_NAME_SIZE)
        {
            return MC_ERR_WRONG_SCRIPT;
        }
        ptr++;
        if(ptr >= ptrEnd)
        {
            return MC_ERR_WRONG_SCRIPT;
        }
    }

    strcpy(name,(char*)ptrStart);
    ptr++;

    *script_size=ptrEnd-ptr;

    if(*script_size)
    {
        memcpy(script,ptr,*script_size);
    }

    return MC_ERR_NOERROR;
}

int mc_Script::SetAssetDetails(const char*name,int multiple,const unsigned char* script,int script_size)
{
    int err;
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+4];

    if((script_size<0) || (script_size>MC_ENT_MAX_SCRIPT_SIZE))
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    err=AddElement();
    if(err)
    {
        return err;
    }

    memcpy(buf,MC_DCT_SCRIPT_FREE_DATA_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_ASSET_DETAILS_PREFIX;

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1);
    if(err)
    {
        return err;
    }

    mc_PutLE(buf,&multiple,4);
    err=SetData(buf,4);
    if(err)
    {
        return err;
    }

    err=SetData((unsigned char*)name,strlen(name)+1);
    if(err)
    {
        return err;
    }

    if(script_size)
    {
        err=SetData(script,script_size);
        if(err)
        {
            return err;
        }
    }

    return MC_ERR_NOERROR;
}

int mc_Script::GetGeneralDetails(unsigned char* script,int *script_size)
{
    unsigned char *ptr;
    unsigned char *ptrEnd;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(m_lpCoord[m_CurrentElement*2+1] > MC_DCT_SCRIPT_IDENTIFIER_LEN+1+MC_ENT_MAX_SCRIPT_SIZE)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];
    ptrEnd=ptr+m_lpCoord[m_CurrentElement*2+1];

    if(memcmp(ptr,MC_DCT_SCRIPT_FREE_DATA_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_GENERAL_DETAILS_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    *script_size=ptrEnd-ptr;

    if(*script_size)
    {
        memcpy(script,ptr,*script_size);
    }

    return MC_ERR_NOERROR;
}

int mc_Script::SetGeneralDetails(const unsigned char* script,int script_size)
{
    int err;
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+4];

    if((script_size<0) || (script_size>MC_ENT_MAX_SCRIPT_SIZE))
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    err=AddElement();
    if(err)
    {
        return err;
    }

    memcpy(buf,MC_DCT_SCRIPT_FREE_DATA_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_GENERAL_DETAILS_PREFIX;

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1);
    if(err)
    {
        return err;
    }

    if(script_size)
    {
        err=SetData(script,script_size);
        if(err)
        {
            return err;
        }
    }

    return MC_ERR_NOERROR;
}

int mc_Script::GetApproval(uint32_t *approval,uint32_t *timestamp)
{
    unsigned char *ptr;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] != MC_DCT_SCRIPT_IDENTIFIER_LEN+1+5)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_APPROVE_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
    *approval=(uint32_t)mc_GetLE(ptr+ 0,1);
    *timestamp=(uint32_t)mc_GetLE(ptr+1,4);

    return MC_ERR_NOERROR;
}


int mc_Script::SetApproval(uint32_t approval,uint32_t timestamp)
{
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+5];
    unsigned char *ptr;
    int err;

    err=AddElement();
    if(err)
    {
        return err;
    }

    ptr=buf;
    memcpy(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_APPROVE_PREFIX;

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
    mc_PutLE(ptr+0,&approval,1);
    mc_PutLE(ptr+1,&timestamp,4);

    return SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1+5);
}

uint32_t mc_GetParamFromDetailsScript(const unsigned char *ptr,uint32_t total,uint32_t offset,uint32_t* param_value_start,size_t *bytes,int *err)
{
    int shift,name_size,value_size,size;

    *param_value_start=0;
    *err=MC_ERR_NOERROR;

    if(offset>=total)
    {
        if(offset > total)
        {
            *err=MC_ERR_ERROR_IN_SCRIPT;
        }
        return total;
    }

    name_size=0;
    if(ptr[offset] == 0x00)
    {
        name_size=2;
    }

    if(name_size == 0)
    {
        name_size=strlen((char*)ptr+offset)+1;
    }

    if(offset+name_size >= total)
    {
        *err=MC_ERR_ERROR_IN_SCRIPT;
        return total;
    }

    value_size=mc_GetVarInt(ptr+offset+name_size,total-offset-name_size,-1,&shift);
    if(value_size<0)
    {
        *err=MC_ERR_ERROR_IN_SCRIPT;
        return total;
    }

    size=name_size+shift+value_size;
    if(offset+size>total)
    {
        *err=MC_ERR_ERROR_IN_SCRIPT;
        return total;
    }

    *bytes=value_size;
    *param_value_start=offset+name_size+shift;

    return offset+size;
}

uint32_t mc_GetParamFromDetailsScript(const unsigned char *ptr,uint32_t total,uint32_t offset,uint32_t* param_value_start,size_t *bytes)
{
    int err;
    return mc_GetParamFromDetailsScript(ptr,total,offset,param_value_start,bytes,&err);
}

int mc_VerifyDetailsScript(const unsigned char *script,uint32_t script_size)
{
    uint32_t offset=0;
    uint32_t new_offset,param_value_start;
    size_t bytes;
    int err;

    while(offset<script_size)
    {
        new_offset=(int32_t)mc_GetParamFromDetailsScript(script,script_size,offset,&param_value_start,&bytes,&err);
        if(err)
        {
            return err;
        }
        offset=new_offset;
    }
    return MC_ERR_NOERROR;
}

int mc_Script::GetNewEntityType(uint32_t *type,int *update,unsigned char* script,int *script_size)
{
    unsigned char *ptr;
    unsigned char *ptrEnd;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1+1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(m_lpCoord[m_CurrentElement*2+1] > MC_DCT_SCRIPT_IDENTIFIER_LEN+1+1+MC_ENT_MAX_SCRIPT_SIZE)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];
    ptrEnd=ptr+m_lpCoord[m_CurrentElement*2+1];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }


    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] == MC_DCT_SCRIPT_HDAC_NEW_ENTITY_PREFIX)
    {
        *update=0;
    }
    else
    {
        if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] == MC_DCT_SCRIPT_HDAC_UPDATE_ENTITY_PREFIX)
        {
            *update=1;
        }
        else
        {
            return MC_ERR_WRONG_SCRIPT;
        }
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    *type=(uint32_t)(*ptr);
    ptr++;

    *script_size=ptrEnd-ptr;

    if(*script_size)
    {
        memcpy(script,ptr,*script_size);
    }

    return mc_VerifyDetailsScript(script,*script_size);
}

int mc_Script::SetNewEntityType(const uint32_t type,const int update,const unsigned char* script,int script_size)
{
    int err;
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+1];

    if((script_size<0) || (script_size>MC_ENT_MAX_SCRIPT_SIZE))
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    err=mc_VerifyDetailsScript(script,script_size);
    if(err)
    {
        return err;
    }

    err=AddElement();
    if(err)
    {
        return err;
    }

    memcpy(buf,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    if(update)
    {
        buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_UPDATE_ENTITY_PREFIX;
    }
    else
    {
        buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_NEW_ENTITY_PREFIX;
    }
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1]=(unsigned char)(type & 0xff);

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1+1);
    if(err)
    {
        return err;
    }

    if(script_size)
    {
        err=SetData(script,script_size);
        if(err)
        {
            return err;
        }
    }

    return MC_ERR_NOERROR;
}

int mc_Script::GetItemKey(unsigned char *key,int *key_size)
{
    unsigned char *ptr;
    unsigned char *ptrEnd;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(m_lpCoord[m_CurrentElement*2+1] > MC_DCT_SCRIPT_IDENTIFIER_LEN+1+MC_ENT_MAX_ITEM_KEY_SIZE)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];
    ptrEnd=ptr+m_lpCoord[m_CurrentElement*2+1];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_KEY_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    *key_size=ptrEnd-ptr;

    if(*key_size)
    {
        memcpy(key,ptr,*key_size);
    }

    return MC_ERR_NOERROR;
}

int mc_Script::SetItemKey(const unsigned char* key,int key_size)
{
    int err;
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+4];

    if((key_size<0) || (key_size>MC_ENT_MAX_ITEM_KEY_SIZE))
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    err=AddElement();
    if(err)
    {
        return err;
    }

    memcpy(buf,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_KEY_PREFIX;

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1);
    if(err)
    {
        return err;
    }

    if(key_size)
    {
        err=SetData(key,key_size);
        if(err)
        {
            return err;
        }
    }

    return MC_ERR_NOERROR;
}


int mc_Script::GetEntity(unsigned char *short_txid)
{
    unsigned char *ptr;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] != MC_DCT_SCRIPT_IDENTIFIER_LEN+1+MC_AST_SHORT_TXID_SIZE)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }
    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_ENTITY_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }
    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    memcpy(short_txid,ptr,MC_AST_SHORT_TXID_SIZE);

    return MC_ERR_NOERROR;
}

int mc_Script::SetEntity(const unsigned char *short_txid)
{
    int err;
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+MC_AST_SHORT_TXID_SIZE];

    err=AddElement();
    if(err)
    {
        return err;
    }

    memcpy(buf,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_ENTITY_PREFIX;
    memcpy(buf+MC_DCT_SCRIPT_IDENTIFIER_LEN+1,short_txid,MC_AST_SHORT_TXID_SIZE);

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1+MC_AST_SHORT_TXID_SIZE);
    if(err)
    {
        return err;
    }

    return MC_ERR_NOERROR;
}

int mc_Script::GetNewEntityType(uint32_t *type)
{
    unsigned char *ptr;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1+1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_NEW_ENTITY_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    *type=(uint32_t)(*ptr);

    return MC_ERR_NOERROR;
}

int mc_Script::SetNewEntityType(const uint32_t type)
{
    int err;
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1+1];

    err=AddElement();
    if(err)
    {
        return err;
    }

    memcpy(buf,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_NEW_ENTITY_PREFIX;
    buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1]=(unsigned char)(type & 0xff);

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1+1);
    if(err)
    {
        return err;
    }

    return MC_ERR_NOERROR;
}


int mc_Script::GetFullRef(unsigned char *ref,uint32_t *script_type)
{
    unsigned char *ptr;
    int items,i,new_ref,shift,ref_type;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }


    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    *script_type=0;

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] == MC_DCT_SCRIPT_HDAC_ASSET_FOLLOWON_PREFIX)
    {
        *script_type=MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON;
    }

    if(*script_type == 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    // TODO : m_AssetRefSize is from mc_HdacParams
    items=(m_lpCoord[m_CurrentElement*2+1] - (MC_DCT_SCRIPT_IDENTIFIER_LEN+1)) / (m_AssetRefSize + MC_AST_ASSET_QUANTITY_SIZE);

    // TODO : m_AssetRefSize is from mc_HdacParams
    if(m_lpCoord[m_CurrentElement*2+1] != MC_DCT_SCRIPT_IDENTIFIER_LEN + 1 + items * (m_AssetRefSize + MC_AST_ASSET_QUANTITY_SIZE))
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    new_ref=1;

    shift=MC_AST_SHORT_TXID_OFFSET;
    ref_type=MC_AST_ASSET_REF_TYPE_SHORT_TXID;

    memset(ref,0,MC_AST_ASSET_FULLREF_SIZE);

    for(i=0;i<items;i++)
    {
        if(new_ref == 0)
        {
            // TODO : m_AssetRefSize is from mc_HdacParams
            if(memcmp(ptr,ref+shift,m_AssetRefSize))
            {
                return MC_ERR_WRONG_SCRIPT;
            }
        }
        else
        {
            // TODO : m_AssetRefSize is from mc_HdacParams
            memcpy(ref+shift,ptr,m_AssetRefSize);
            new_ref=0;
        }
        // TODO : m_AssetRefSize is from mc_HdacParams
        ptr+=m_AssetRefSize + MC_AST_ASSET_QUANTITY_SIZE;
    }

    if(new_ref == 0)
    {
        mc_SetABRefType(ref,ref_type);
    }

    return MC_ERR_NOERROR;
}

int mc_Script::GetAssetQuantities(mc_Buffer *amounts,uint32_t script_type)
{
    unsigned char *ptr;
    int items,i,row,shift,ref_type;
    int64_t last,quantity;
    uint32_t found_script_type,last_script_type;
    unsigned char buf[MC_AST_ASSET_FULLREF_BUF_SIZE];
    int valid_identitfier;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }


    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    valid_identitfier=0;
    if(script_type & MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER)
    {
        if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] == MC_DCT_SCRIPT_HDAC_ASSET_QUANTITY_PREFIX)
        {
            valid_identitfier=1;
            found_script_type=MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER;
        }
    }

    if(script_type & MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON)
    {
        if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] == MC_DCT_SCRIPT_HDAC_ASSET_FOLLOWON_PREFIX)
        {
            valid_identitfier=1;
            found_script_type=MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON;
        }
    }

    if(valid_identitfier == 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr+=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;

    shift=MC_AST_SHORT_TXID_OFFSET;
    ref_type=MC_AST_ASSET_REF_TYPE_SHORT_TXID;

    // TODO : m_AssetRefSize is from mc_HdacParams
    items=(m_lpCoord[m_CurrentElement*2+1] - (MC_DCT_SCRIPT_IDENTIFIER_LEN+1)) / (m_AssetRefSize + MC_AST_ASSET_QUANTITY_SIZE);

    // TODO : m_AssetRefSize is from mc_HdacParams
    if(m_lpCoord[m_CurrentElement*2+1] != MC_DCT_SCRIPT_IDENTIFIER_LEN + 1 + items * (m_AssetRefSize + MC_AST_ASSET_QUANTITY_SIZE))
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    for(i=0;i<items;i++)
    {
        // TODO : m_AssetRefSize is from mc_HdacParams
        quantity=mc_GetLE(ptr+m_AssetRefSize,MC_AST_ASSET_QUANTITY_SIZE);
        if(quantity < 0)
        {
            return MC_ERR_NOT_ALLOWED;
        }
        memset(buf,0, MC_AST_ASSET_FULLREF_BUF_SIZE);

        // TODO : m_AssetRefSize is from mc_HdacParams
        memcpy(buf+shift,ptr,m_AssetRefSize);
        mc_SetABRefType(buf,ref_type);

        row=amounts->Seek(buf);
        last=0;
        if(row >= 0)
        {
            last=mc_GetABQuantity(amounts->GetRow(row));
            last_script_type=(uint32_t)mc_GetABScriptType(amounts->GetRow(row));
/*
            if(quantity+last < 0)
            {
                return MC_ERR_NOT_ALLOWED;
            }
 */
            quantity+=last;
            if(last >= 0)
            {
                if(quantity < 0)                                                    // Protection from overflow
                {
                    return MC_ERR_NOT_ALLOWED;
                }
            }
            found_script_type|=last_script_type;
            mc_SetABQuantity(amounts->GetRow(row),quantity);
            mc_SetABScriptType(amounts->GetRow(row),found_script_type);
        }
        else
        {
            mc_SetABQuantity(buf,quantity);
            mc_SetABScriptType(buf,found_script_type);
            amounts->Add(buf);
        }

        // TODO : m_AssetRefSize is from mc_HdacParams
        ptr+=m_AssetRefSize + MC_AST_ASSET_QUANTITY_SIZE;
    }

    return MC_ERR_NOERROR;
}

int mc_Script::SetAssetQuantities(mc_Buffer *amounts,uint32_t script_type)
{
    unsigned char buf[MC_DCT_SCRIPT_IDENTIFIER_LEN+1];
    unsigned char *ptr;
    int err,i,shift;

    err=AddElement();
    if(err)
    {
        return err;
    }

    ptr=buf;
    memcpy(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);

    switch(script_type)
    {
        case MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER:
            ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_ASSET_QUANTITY_PREFIX;
            break;
        case MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON:
            ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_ASSET_FOLLOWON_PREFIX;
            break;
        default:
            return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1);
    if(err)
    {
        return err;
    }

    shift=0;
    shift=MC_AST_SHORT_TXID_OFFSET;


    for(i=0;i<amounts->GetCount();i++)
    {
        // TODO : m_AssetRefSize is from mc_HdacParams
        err=SetData(amounts->GetRow(i)+shift,m_AssetRefSize);
        if(err)
        {
            return err;
        }
        err=SetData(amounts->GetRow(i)+MC_AST_ASSET_QUANTITY_OFFSET,MC_AST_ASSET_QUANTITY_SIZE);
        if(err)
        {
            return err;
        }
    }

    return MC_ERR_NOERROR;
}

int mc_Script::GetCachedScript(int offset, int *next_offset, int* vin, unsigned char** script, int *script_size)
{
    unsigned char *ptr;
    unsigned char *ptrEnd;
    int shift;

    if(m_CurrentElement<0)
    {
        return MC_ERR_INVALID_PARAMETER_VALUE;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < MC_DCT_SCRIPT_IDENTIFIER_LEN+1)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    ptr=m_lpData+m_lpCoord[m_CurrentElement*2+0];

    if(memcmp(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN) != 0)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN] != MC_DCT_SCRIPT_HDAC_CASHED_SCRIPT_PREFIX)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(offset == 0)
    {
        *next_offset=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
        return MC_ERR_NOERROR;
    }

    if(m_lpCoord[m_CurrentElement*2+1] == offset)
    {
        return MC_ERR_WRONG_SCRIPT;
    }

    if(m_lpCoord[m_CurrentElement*2+1] < offset + 4)
    {
        return MC_ERR_ERROR_IN_SCRIPT;
    }

    ptrEnd=ptr+m_lpCoord[m_CurrentElement*2+1];
    ptr+=offset;

    *vin=mc_GetLE(ptr,4);
    if(*vin<0)
    {
        return MC_ERR_ERROR_IN_SCRIPT;
    }
    ptr+=4;

    *script_size=(int)mc_GetVarInt(ptr,ptrEnd-ptr,-1,&shift);
    if(*script_size<0)
    {
        return MC_ERR_ERROR_IN_SCRIPT;
    }

    ptr+=shift;

    if(ptr+*script_size > ptrEnd)
    {
        return MC_ERR_ERROR_IN_SCRIPT;
    }

    *script=ptr;
    *next_offset=offset+4+shift+*script_size;

    return MC_ERR_NOERROR;
}

int mc_Script::SetCachedScript(int offset, int *next_offset, int vin, unsigned char* script, int script_size)
{
    unsigned char buf[16];
    unsigned char *ptr;
    int err,shift;

    if(offset == 0)
    {
        err=AddElement();
        if(err)
        {
            return err;
        }

        ptr=buf;
        memcpy(ptr,MC_DCT_SCRIPT_HDAC_IDENTIFIER,MC_DCT_SCRIPT_IDENTIFIER_LEN);

        ptr[MC_DCT_SCRIPT_IDENTIFIER_LEN]=MC_DCT_SCRIPT_HDAC_CASHED_SCRIPT_PREFIX;

        err=SetData(buf,MC_DCT_SCRIPT_IDENTIFIER_LEN+1);
        if(err)
        {
            return err;
        }
        *next_offset=MC_DCT_SCRIPT_IDENTIFIER_LEN+1;
        return MC_ERR_NOERROR;
    }

    mc_PutLE(buf,&vin,4);
    err=SetData(buf,4);
    if(err)
    {
        return err;
    }

    shift=mc_PutVarInt(buf,16,script_size);
    err=SetData(buf,shift);
    if(err)
    {
        return err;
    }

    err=SetData(script,script_size);
    if(err)
    {
        return err;
    }

    *next_offset=offset+4+shift+script_size;

    return MC_ERR_NOERROR;
}

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


uint32_t mc_FindSpecialParamInDetailsScript(const unsigned char *ptr,uint32_t total,uint32_t param,size_t *bytes)
{
    uint32_t offset,new_offset;
    uint32_t value_offset;
    size_t value_size;

    offset=0;
    while(offset<total)
    {
        new_offset=mc_GetParamFromDetailsScript(ptr,total,offset,&value_offset,&value_size);
        if(value_offset > 0)
        {
            if(ptr[offset] == 0x00)
            {
                if(ptr[offset+1] == (unsigned char)param)
                {
                    *bytes=value_size;
                    return value_offset;
                }
            }
        }
        offset=new_offset;
    }

    return total;
}

uint32_t mc_FindNamedParamInDetailsScript(const unsigned char *ptr,uint32_t total,const char *param,size_t *bytes)
{
    uint32_t offset,new_offset;
    uint32_t value_offset;
    size_t value_size;

    offset=0;
    while(offset<total)
    {
        new_offset=mc_GetParamFromDetailsScript(ptr,total,offset,&value_offset,&value_size);
        if(value_offset > 0)
        {
            if( ptr[offset] != 0x00 )
            {
                if( strlen(param) == strlen((char*)ptr+offset) )
                {
                    if(mc_StringCompareCaseInsensitive(param,(char*)ptr+offset,strlen(param)) == 0)
                    {
                        *bytes=value_size;
                        return value_offset;
                    }
                }
            }
        }
        offset=new_offset;
    }

    return total;
}

