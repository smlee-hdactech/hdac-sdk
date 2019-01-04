#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include <json_spirit/json_spirit.h>
#include <string>
#include <memory>
#include <structs/uint256.h>
#include "standard.h"

class mc_EntityDetails;

#define MC_ASSET_KEY_UNCONFIRMED_GENESIS    1
#define MC_ASSET_KEY_VALID                  0
#define MC_ASSET_KEY_INVALID_TXID          -1
#define MC_ASSET_KEY_INVALID_REF           -2
#define MC_ASSET_KEY_INVALID_NAME          -3
#define MC_ASSET_KEY_INVALID_SIZE          -4
#define MC_ASSET_KEY_INVALID_EMPTY         -5


bool AssetRefDecode(unsigned char *bin, const char* string, const size_t stringLen);
void ParseEntityIdentifier(json_spirit::Value entity_identifier,mc_EntityDetails *entity,uint32_t entity_type);

class IPrivateKeyHelper;
class CKey;
CKey keyFromPrivateKey(const std::string& privateKey, const IPrivateKeyHelper& helper);

class CScript;
bool solver(const std::string& privateKey, const IPrivateKeyHelper& helper, const CScript& scriptPubKey,
            uint256 hash, int nHashType, const std::string& unspentRedeemScript,
            CScript& scriptSigRet, txnouttype& whichTypeRet);

bool ExtractDestinationScriptValid(const CScript& scriptPubKey, CTxDestination& addressRet);

#endif // TRANSACTIONS_H
