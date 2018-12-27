#include "asset.h"
#include <cstring>
#include <utils/utility.h>
#include <script/hdacscript.h>
#include <utils/define.h>

#define MC_AST_ASSET_REF_TYPE_OFFSET        32
#define MC_AST_ASSET_REF_TYPE_SIZE           4
#define MC_AST_ASSET_SCRIPT_TYPE_OFFSET     44
#define MC_AST_ASSET_SCRIPT_TYPE_SIZE        4

void mc_EntityLedgerRow::Zero()
{
    memset(this,0,sizeof(mc_EntityLedgerRow));
}

void mc_EntityDetails::Zero()
{
    memset(this,0,sizeof(mc_EntityDetails));
    m_LedgerRow.Zero();
}

const char* mc_EntityDetails::GetName()
{
    uint32_t value_offset;
    size_t value_size;
    unsigned char dname_buf[6];

    if(m_LedgerRow.m_ScriptSize)
    {
        value_offset=mc_FindSpecialParamInDetailsScript(m_LedgerRow.m_Script,m_LedgerRow.m_ScriptSize,MC_ENT_SPRM_NAME,&value_size);
        if(value_offset == m_LedgerRow.m_ScriptSize)
        {
            strcpy((char*)dname_buf+1,"name");
            dname_buf[0]=0xff;
            value_offset=mc_FindNamedParamInDetailsScript(m_LedgerRow.m_Script,m_LedgerRow.m_ScriptSize,(char*)dname_buf,&value_size);
        }
        if(value_offset < m_LedgerRow.m_ScriptSize)
        {
            if(value_size == 2)
            {
                if((char)m_LedgerRow.m_Script[value_offset] == '*')
                {
                    value_offset=m_LedgerRow.m_ScriptSize;
                    value_size=0;
                }
            }
        }
        if(value_offset < m_LedgerRow.m_ScriptSize)
        {
            return (char*)(m_LedgerRow.m_Script+value_offset);
        }
    }

    return m_Name;
}

const unsigned char* mc_EntityDetails::GetTxID()
{
    return m_LedgerRow.m_Key;
}

const unsigned char* mc_EntityDetails::GetRef()
{
    return m_Ref;
}

int mc_EntityDetails::IsUnconfirmedGenesis()
{
    return ((int)mc_GetLE(m_Ref+4,4)<0) ? 1 : 0;
}

const unsigned char* mc_EntityDetails::GetFullRef()
{
    return m_FullRef;
}

const unsigned char* mc_EntityDetails::GetShortRef()
{
    return GetTxID()+MC_AST_SHORT_TXID_OFFSET;
}

const unsigned char* mc_EntityDetails::GetScript()
{
    return m_LedgerRow.m_Script;
}

int mc_EntityDetails::GetAssetMultiple()
{
    int multiple;
    size_t size;
    void* ptr;

    multiple=1;

    ptr=NULL;
    ptr=(void*)GetSpecialParam(MC_ENT_SPRM_ASSET_MULTIPLE,&size);

    if(ptr)
    {
        if(size==sizeof(multiple))
        {
            multiple=(int)mc_GetLE(ptr,size);
        }
    }

    if(multiple <= 0)
    {
        multiple=1;
    }

    return multiple;
}

int mc_EntityDetails::IsFollowOn()
{
    if(m_LedgerRow.m_KeyType & MC_ENT_KEYTYPE_FOLLOW_ON)
    {
        return 1;
    }
    return 0;
}

int mc_EntityDetails::AllowedFollowOns()
{
    unsigned char *ptr;
    size_t bytes;
    ptr=(unsigned char *)GetSpecialParam(MC_ENT_SPRM_FOLLOW_ONS,&bytes);
    if(ptr)
    {
        if((bytes>0) && (bytes<=4))
        {
            return (int)mc_GetLE(ptr,bytes);
        }
    }
    return 0;
}

int mc_EntityDetails::AnyoneCanWrite()
{
    unsigned char *ptr;
    size_t bytes;
    ptr=(unsigned char *)GetSpecialParam(MC_ENT_SPRM_ANYONE_CAN_WRITE,&bytes);
    if(ptr)
    {
        if((bytes>0) && (bytes<=4))
        {
            return (int)mc_GetLE(ptr,bytes);
        }
    }
    return 0;
}

uint32_t mc_EntityDetails::UpgradeStartBlock()
{
    unsigned char *ptr;
    size_t bytes;
    ptr=(unsigned char *)GetSpecialParam(MC_ENT_SPRM_UPGRADE_START_BLOCK,&bytes);
    if(ptr)
    {
        if((bytes>0) && (bytes<=4))
        {
            return (int)mc_GetLE(ptr,bytes);
        }
    }
    return 0;
}

int mc_EntityDetails::UpgradeProtocolVersion()
{
    unsigned char *ptr;
    size_t bytes;
    ptr=(unsigned char *)GetSpecialParam(MC_ENT_SPRM_UPGRADE_PROTOCOL_VERSION,&bytes);
    if(ptr)
    {
        if((bytes>0) && (bytes<=4))
        {
            return (int)mc_GetLE(ptr,bytes);
        }
    }
    return 0;
}


uint64_t mc_EntityDetails::GetQuantity()
{
    return m_LedgerRow.m_Quantity;
}

uint32_t mc_EntityDetails::GetEntityType()
{
    return m_LedgerRow.m_EntityType;
}

int32_t mc_EntityDetails::NextParam(uint32_t offset,uint32_t* param_value_start,size_t *bytes)
{
    int32_t new_offset;

    new_offset=(int32_t)mc_GetParamFromDetailsScript(m_LedgerRow.m_Script,m_LedgerRow.m_ScriptSize,offset,param_value_start,bytes);
    if(new_offset == (int32_t)m_LedgerRow.m_ScriptSize)
    {
        new_offset = -1;
    }

    return new_offset;
}

const void* mc_EntityDetails::GetSpecialParam(uint32_t param,size_t* bytes)
{
    uint32_t offset;
    offset=mc_FindSpecialParamInDetailsScript(m_LedgerRow.m_Script,m_LedgerRow.m_ScriptSize,param,bytes);
    if(offset == m_LedgerRow.m_ScriptSize)
    {
        return NULL;
    }
    return m_LedgerRow.m_Script+offset;
}

const void* mc_EntityDetails::GetParam(const char *param,size_t* bytes)
{
    uint32_t offset;
    offset=mc_FindNamedParamInDetailsScript(m_LedgerRow.m_Script,m_LedgerRow.m_ScriptSize,param,bytes);
    if(offset == m_LedgerRow.m_ScriptSize)
    {
        return NULL;
    }
    return m_LedgerRow.m_Script+offset;
}

uint32_t mc_GetABScriptType(void *ptr)
{
    return (uint32_t)mc_GetLE((unsigned char*)ptr+MC_AST_ASSET_SCRIPT_TYPE_OFFSET,MC_AST_ASSET_SCRIPT_TYPE_SIZE);
}

void mc_SetABScriptType(void *ptr,uint32_t type)
{
    mc_PutLE((unsigned char*)ptr+MC_AST_ASSET_SCRIPT_TYPE_OFFSET,&type,MC_AST_ASSET_SCRIPT_TYPE_SIZE);
}

uint32_t mc_GetABRefType(void *ptr)
{
    return (uint32_t)mc_GetLE((unsigned char*)ptr+MC_AST_ASSET_REF_TYPE_OFFSET,MC_AST_ASSET_REF_TYPE_SIZE);
}

void mc_SetABRefType(void *ptr,uint32_t type)
{
    mc_PutLE((unsigned char*)ptr+MC_AST_ASSET_REF_TYPE_OFFSET,&type,MC_AST_ASSET_REF_TYPE_SIZE);
}

int64_t mc_GetABQuantity(void *ptr)
{
    return (int64_t)mc_GetLE((unsigned char*)ptr+MC_AST_ASSET_QUANTITY_OFFSET,MC_AST_ASSET_QUANTITY_SIZE);
}

void mc_SetABQuantity(void *ptr,int64_t quantity)
{
    mc_PutLE((unsigned char*)ptr+MC_AST_ASSET_QUANTITY_OFFSET,&quantity,MC_AST_ASSET_QUANTITY_SIZE);
}

void mc_InitABufferDefault(mc_Buffer *buf)
{
    buf->Initialize(MC_AST_ASSET_QUANTITY_OFFSET,MC_AST_ASSET_FULLREF_BUF_SIZE,MC_BUF_MODE_DEFAULT);
}
