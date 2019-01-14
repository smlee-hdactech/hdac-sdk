﻿#ifndef HDACSCRIPT_H
#define HDACSCRIPT_H

#include <cstddef>
#include <cstdint>

#define MC_SCR_TYPE_SCRIPTPUBKEY                      0
#define MC_SCR_TYPE_SCRIPTSIG                         1
#define MC_SCR_TYPE_SCRIPTSIGRAW                      2

#define MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER              0x00000001
#define MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON              0x00000002

struct mc_Buffer;

typedef struct mc_Script
{
    int m_Size;
    int m_NumElements;
    int m_CurrentElement;
    unsigned char* m_lpData;
    int *m_lpCoord;
    int m_AllocElements;
    int m_AllocSize;
    int m_ScriptType;

    mc_Script()
    {
        Zero();
    }

    ~mc_Script()
    {
        Destroy();
    }

    int Zero();
    int Destroy();
    int Resize(size_t bytes,int elements);

    int SetScript(const unsigned char* src,const size_t bytes,int type);
    int IsOpReturnScript();
    int IsDirtyOpReturnScript();
    int Clear();

    int GetNumElements();
    int AddElement();
    int SetSpecialParamValue(unsigned char param,const unsigned char* param_value,const size_t param_value_size);
    int SetParamValue(const char *param_name,const size_t param_name_size,const unsigned char* param_value,const size_t param_value_size);
    size_t GetParamValue(const unsigned char *ptr,size_t total,size_t offset,size_t* param_value_start,size_t *bytes);
    int SetData(const unsigned char* src,const size_t bytes);
    const unsigned char* GetData(int element,size_t *bytes);

    int GetElement();
    int SetElement(int element);

    int GetEntity(unsigned char *short_txid);
    int SetEntity(const unsigned char *short_txid);

    int GetNewEntityType(uint32_t *type);
    int SetNewEntityType(const uint32_t type);

    int GetApproval(uint32_t *approval,uint32_t *timestamp);
    int SetApproval( uint32_t approval,uint32_t timestamp);

    int GetNewEntityType(uint32_t *type,int *update,unsigned char* script,int *script_size);
    int SetNewEntityType(const uint32_t type,const int update,const unsigned char* script,int script_size);

    int GetItemKey(unsigned char *key,int *key_size);
    int SetItemKey(const unsigned char* key,int key_size);

    int GetPermission(uint32_t *type,uint32_t *from,uint32_t *to,uint32_t *timestamp);
    int SetPermission(uint32_t type,uint32_t from,uint32_t to,uint32_t timestamp);

    int GetBlockSignature(unsigned char* sig,int *sig_size,uint32_t *hash_type,unsigned char* key,int *key_size);
    int SetBlockSignature(const unsigned char* sig,int sig_size,uint32_t hash_type,const unsigned char* key,int key_size);

    int GetAssetGenesis(int64_t *quantity);
    int SetAssetGenesis(int64_t quantity);

    int GetAssetDetails(char* name,int* multiple,unsigned char* script,int *script_size);
    int SetAssetDetails(const char*name,int multiple,const unsigned char* script,int script_size);

    int GetGeneralDetails(unsigned char* script,int *script_size);
    int SetGeneralDetails(const unsigned char* script,int script_size);

    int GetAssetQuantities(mc_Buffer *amounts,uint32_t script_type);
    int SetAssetQuantities(mc_Buffer *amounts,uint32_t script_type);

    int GetFullRef(unsigned char *ref,uint32_t *script_type);

    int GetCachedScript(int offset, int *next_offset, int* vin, unsigned char** script, int *script_size);
    int SetCachedScript(int offset, int *next_offset, int vin, unsigned char* script, int script_size);

} mc_Script;

const unsigned char *mc_ParseOpDropOpReturnScript(const unsigned char *src,int size,int *op_drop_offset,int *op_drop_size,int op_drop_count,int *op_return_offset,int *op_return_size);
uint32_t mc_FindSpecialParamInDetailsScript(const unsigned char *ptr,uint32_t total,uint32_t param,size_t *bytes);
uint32_t mc_FindNamedParamInDetailsScript(const unsigned char *ptr,uint32_t total,const char *param,size_t *bytes);
uint32_t mc_GetParamFromDetailsScript(const unsigned char *ptr,uint32_t total,uint32_t offset,uint32_t* param_value_start,size_t *bytes);

#endif // HDACSCRIPT_H
