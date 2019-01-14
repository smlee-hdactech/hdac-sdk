#ifndef ASSET_H
#define ASSET_H

#include <cstdint>
#include <cstddef>

#define MC_AST_ASSET_REF_SIZE        10
#define MC_AST_ASSET_BUF_TOTAL_SIZE  22
#define MC_AST_SHORT_TXID_OFFSET     16
#define MC_AST_SHORT_TXID_SIZE       16

#define MC_AST_ASSET_BUFFER_REF_SIZE        32
#define MC_AST_ASSET_FULLREF_SIZE           36
#define MC_AST_ASSET_QUANTITY_OFFSET        36
#define MC_AST_ASSET_QUANTITY_SIZE           8
#define MC_AST_ASSET_FULLREF_BUF_SIZE       48

#define MC_AST_ASSET_REF_TYPE_REF            0
#define MC_AST_ASSET_REF_TYPE_SHORT_TXID     1
#define MC_AST_ASSET_REF_TYPE_TXID           2

#define MC_ENT_REF_SIZE                 10
#define MC_ENT_REF_PREFIX_SIZE           2
#define MC_ENT_MAX_NAME_SIZE            32
#define MC_ENT_MAX_ITEM_KEY_SIZE       256
#define MC_ENT_MAX_SCRIPT_SIZE        4096
#define MC_ENT_MAX_FIXED_FIELDS_SIZE   128
#define MC_ENT_MAX_STORED_ISSUERS      128
#define MC_ENT_SCRIPT_ALLOC_SIZE      8192 // > MC_ENT_MAX_SCRIPT_SIZE + MC_ENT_MAX_FIXED_FIELDS_SIZE + 27*MC_ENT_MAX_STORED_ISSUERS

#define MC_ENT_KEY_SIZE              32
#define MC_ENT_KEYTYPE_TXID           0x00000001
#define MC_ENT_KEYTYPE_REF            0x00000002
#define MC_ENT_KEYTYPE_NAME           0x00000003
#define MC_ENT_KEYTYPE_SHORT_TXID     0x00000004
#define MC_ENT_KEYTYPE_MASK           0x000000FF
#define MC_ENT_KEYTYPE_FOLLOW_ON      0x00000100

#define MC_ENT_TYPE_ANY               0xFF
#define MC_ENT_TYPE_NONE              0x00
#define MC_ENT_TYPE_ASSET             0x01
#define MC_ENT_TYPE_STREAM            0x02
#define MC_ENT_TYPE_STREAM_MAX        0x0F
#define MC_ENT_TYPE_UPGRADE           0x10
#define MC_ENT_TYPE_MAX               0x10

#define MC_ENT_SPRM_NAME                      0x01
#define MC_ENT_SPRM_FOLLOW_ONS                0x02
#define MC_ENT_SPRM_ISSUER                    0x03
#define MC_ENT_SPRM_ANYONE_CAN_WRITE          0x04
#define MC_ENT_SPRM_JSON_DETAILS              0x05
#define MC_ENT_SPRM_ASSET_MULTIPLE            0x41
#define MC_ENT_SPRM_UPGRADE_PROTOCOL_VERSION  0x42
#define MC_ENT_SPRM_UPGRADE_START_BLOCK       0x43

/** Ledger and mempool record structure */

typedef struct mc_EntityLedgerRow
{
    unsigned char m_Key[MC_ENT_KEY_SIZE];                                       // Entity key size - txid/entity-ref/name
    uint32_t m_KeyType;                                                         // Entity key type - MC_ENT_KEYTYPE_ constants
    int32_t m_Block;                                                            // Block entity is confirmed in
    int32_t m_Offset;                                                           // Offset of the entity in the block
    uint32_t m_ScriptSize;                                                      // Script Size
    int64_t m_Quantity;                                                         // Total quantity of the entity (including follow-ons)
    uint32_t m_EntityType;                                                      // Entity type - MC_ENT_TYPE_ constants
    uint32_t m_Reserved1;                                                       // Reserved to align to 96 bytes
    int64_t m_PrevPos;                                                          // Position of the previous entity in the ledger
    int64_t m_FirstPos;                                                         // Position in the ledger corresponding to first object in the chain
    int64_t m_LastPos;                                                          // Position in the ledger corresponding to last object in the chain before this object
    int64_t m_ChainPos;                                                         // Position in the ledger corresponding to last object in the chain
    unsigned char m_Script[MC_ENT_SCRIPT_ALLOC_SIZE];                           // Script > MC_ENT_MAX_SCRIPT_SIZE + MC_ENT_MAX_FIXED_FIELDS_SIZE + 27*MC_ENT_MAX_STORED_ISSUERS

    void Zero();
} mc_EntityLedgerRow;

/** Entity details structure */

typedef struct mc_EntityDetails
{
    unsigned char m_Ref[MC_ENT_REF_SIZE];                                       // Entity reference
    unsigned char m_FullRef[MC_AST_ASSET_QUANTITY_OFFSET];                      // Full Entity reference, derived from short txid from v 10007
    char m_Name[MC_ENT_MAX_NAME_SIZE+6];                                        // Entity name
    uint32_t m_Flags;
    unsigned char m_Reserved[36];
    mc_EntityLedgerRow m_LedgerRow;
    void Zero();
    void Set(mc_EntityLedgerRow *row);
    const char* GetName();
    const unsigned char* GetTxID();
    const unsigned char* GetRef();
    const unsigned char* GetFullRef();
    const unsigned char* GetShortRef();
    const unsigned char* GetScript();
    int IsUnconfirmedGenesis();
    int GetAssetMultiple();
    int IsFollowOn();
//    int HasFollowOns();
    int AllowedFollowOns();
    int AnyoneCanWrite();
    int UpgradeProtocolVersion();
    uint32_t UpgradeStartBlock();
    uint64_t GetQuantity();
    uint32_t GetEntityType();
    const void* GetSpecialParam(uint32_t param,size_t* bytes);
    const void* GetParam(const char *param,size_t* bytes);
    int32_t NextParam(uint32_t offset,uint32_t* param_value_start,size_t *bytes);
}mc_EntityDetails;

uint32_t mc_GetABScriptType(void *ptr);
void mc_SetABScriptType(void *ptr,uint32_t type);
uint32_t mc_GetABRefType(void *ptr);
void mc_SetABRefType(void *ptr,uint32_t type);
int64_t mc_GetABQuantity(void *ptr);
void mc_SetABQuantity(void *ptr,int64_t quantity);
struct mc_Buffer;
void mc_InitABufferDefault(mc_Buffer *buf);

// TODO : m_AssetRefSize is from mc_HdacParams
static int m_AssetRefSize = MC_AST_SHORT_TXID_SIZE;

#endif // ASSET_H
