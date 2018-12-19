#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include <json_spirit/json_spirit.h>
#include <string>

class IWalletAddrHelper;
class mc_EntityDetails;

#define MC_ASSET_KEY_UNCONFIRMED_GENESIS    1
#define MC_ASSET_KEY_VALID                  0
#define MC_ASSET_KEY_INVALID_TXID          -1
#define MC_ASSET_KEY_INVALID_REF           -2
#define MC_ASSET_KEY_INVALID_NAME          -3
#define MC_ASSET_KEY_INVALID_SIZE          -4
#define MC_ASSET_KEY_INVALID_EMPTY         -5


bool AssetRefDecode(unsigned char *bin, const char* string, const size_t stringLen);
int ParseAssetKeyToFullAssetRef(const char* asset_key,unsigned char *full_asset_ref,int *multiple,int *type,int entity_type);

json_spirit::Value createMultisigStreamTx(const std::string &strAddr, const std::string& streamName,
                                          const std::string& streamKey, const std::string& streamItem,
                                          const std::string &createTxid,
                                          const std::string &unspentScriptPubKey, const std::string &unspentTxid, uint32_t unspentVOut,
                                          const std::string &unspentRedeemScript, const std::string &privateKey);

#if 0
json_spirit::Value createrawsendfrom(const std::string &strAddr, const std::string &jsonAddrOrAmount,
                                     const std::string &jsonAppendRawdata,
                                     const IWalletAddrHelper& helper);
#endif
void ParseEntityIdentifier(json_spirit::Value entity_identifier,mc_EntityDetails *entity,uint32_t entity_type);

#endif // TRANSACTIONS_H
