#include "transactions.h"
#include <json_spirit/json_spirit.h>
#include <set>
#include "bitcoinaddress.h"
#include "key.h"
#include <json_spirit/json_spirit_reader_template.h>
#include <structs/amount.h>
#include "multisig.h"
#include <utils/tinyformat.h>
#include "bitcoinsecret.h"
#include <entities/asset.h>
#include "eccautoinitreleasehandler.h"

// TODO : remove dependencies with rpc module
#include <rpc/rpcprotocol.h>


using namespace std;
using namespace json_spirit;

vector<CTxDestination> ParseAddresses(string param, bool allow_scripthash, const IWalletAddrHelper &helper)
{
    vector<CTxDestination> addresses;
    //set<CTxDestination> thisAddresses;
    set<CBitcoinAddress> setAddress;

    string tok;

    //CKeyID *lpKeyID;

    stringstream ss(param);
    while(getline(ss, tok, ','))
    {
        CBitcoinAddress address(tok, helper);
        if (!address.IsValid())
            throw "Invalid address: "+tok;
        if (setAddress.count(address))
            throw "Invalid parameter, duplicated address: "+tok;

        CTxDestination dest=address.Get();
        CKeyID *lpKeyID=boost::get<CKeyID> (&dest);
        CScriptID *lpScriptID=boost::get<CScriptID> (&dest);

        if( (lpKeyID != NULL) || (( lpScriptID != NULL) && allow_scripthash ))
        {
            addresses.push_back(address.Get());
            setAddress.insert(address);
        }
        else
        {
            if(allow_scripthash)
            {
                throw "Invalid address (only pubkeyhash and scripthash addresses are supported) : "+tok;
            }
            else
            {
                throw "Invalid address (only pubkeyhash addresses are supported) : "+tok;
            }
        }
    }

    return addresses;
}

CScript GetScriptForPubKey(const CPubKey& key)
{
    CScript script;
    script << ToByteVector(key);
    script << OP_CHECKSIG;

    return script;
}

// TODO : replace
static inline int64_t roundint64(double d)
{
    return (int64_t)(d > 0 ? d + 0.5 : d - 0.5);
}

CAmount AmountFromValue(const Value& value)
{
    double dAmount = value.get_real();

    if(COIN == 0)
    {
        if(dAmount != 0)
        {
            throw "Invalid amount";
        }
    }
    else
    {
        if (dAmount < 0.0 || dAmount > (double)MAX_MONEY/(double)COIN)
            throw "Invalid amount";
    }

    CAmount nAmount = roundint64(dAmount * COIN);
    if (!MoneyRange(nAmount))
        throw "Invalid amount";
    return nAmount;
}

bool AssetRefDecode(unsigned char *bin, const char* string, const size_t stringLen)
{
    char buffer[1024];
    int txIDPrefixInteger;
    long long blockNum, txOffset;

    if(stringLen <= 0)
        return false;

    if (stringLen>=sizeof(buffer))
        return false;

    memcpy(buffer, string, stringLen);
    buffer[stringLen]=0; // copy to our buffer and null terminate to allow scanf

    if (strchr(buffer, '+')) // special check for '+' character which would be accepted by sscanf() below
        return false;

    if (sscanf(buffer, "%lld-%lld-%d", &blockNum, &txOffset, &txIDPrefixInteger)!=3)
        return false;

    if ( (txIDPrefixInteger<0) || (txIDPrefixInteger>0xFFFF) )
        return false;

    mc_PutLE(bin+0,&blockNum,4);
    mc_PutLE(bin+4,&txOffset,8);
    bin[8]=(unsigned char)(txIDPrefixInteger%256);
    bin[9]=(unsigned char)(txIDPrefixInteger/256);

    return true;
}

int ParseAssetKey(const char* asset_key,unsigned char *txid,unsigned char *asset_ref,char *name,int *multiple,int *type,int entity_type)
{
    int ret=MC_ASSET_KEY_VALID;
    int size=strlen(asset_key);
    unsigned char buf[MC_AST_ASSET_REF_SIZE];
    mc_EntityDetails entity;
    const unsigned char *ptr;

    if(size == 0)
    {
        return MC_ASSET_KEY_INVALID_EMPTY;
    }

    uint256 hash=0;
    if(size == 64)      // maybe txid
    {
        if(type)
        {
            *type=MC_ENT_KEYTYPE_TXID;
        }
        hash.SetHex(asset_key);

#if 0
        if(!mc_gState->m_Assets->FindEntityByTxID(&entity,(unsigned char*)&hash))
        {
            ret=MC_ASSET_KEY_INVALID_TXID;
        }
        else
        {
            if( (entity_type != MC_ENT_TYPE_ANY) && ((int)entity.GetEntityType() != entity_type) )
            {
                ret=MC_ASSET_KEY_INVALID_TXID;
            }
            else
            {
                int root_stream_name_size;
                mc_gState->m_NetworkParams->GetParam("rootstreamname",&root_stream_name_size);
                if(root_stream_name_size <= 1)
                {
                    if(hash == mc_GenesisCoinbaseTxID())
                    {
                        ret=MC_ASSET_KEY_INVALID_TXID;
                    }
                }
            }
            if(entity.IsFollowOn())
            {
                if(!mc_gState->m_Assets->FindEntityByFollowOn(&entity,(unsigned char*)&hash))
                {
                    ret=MC_ASSET_KEY_INVALID_TXID;
                }
            }
        }
#else
        // TODO: how to implement
        assert(!"how to implement");
#endif
    }
    else
    {
        if(size<=MC_ENT_MAX_NAME_SIZE)
        {
            if(AssetRefDecode(buf,asset_key,size))
            {
                if(type)
                {
                    *type=MC_ENT_KEYTYPE_REF;
                }
#if 0
                if(!mc_gState->m_Assets->FindEntityByRef(&entity,buf))
                {
                    ret=MC_ASSET_KEY_INVALID_REF;
                }
                else
                {
                    if( (entity_type != MC_ENT_TYPE_ANY) && ((int)entity.GetEntityType() != entity_type) )
                    {
                        ret=MC_ASSET_KEY_INVALID_REF;
                    }
                }
#else
                // TODO : how to implement
                assert(!"how to implement");
#endif
            }
            else
            {
                if(type)
                {
                    *type=MC_ENT_KEYTYPE_NAME;
                }
#if 0
                if(!mc_gState->m_Assets->FindEntityByName(&entity,(char*)asset_key))
                {
                    ret=MC_ASSET_KEY_INVALID_NAME;
                }
                else
                {
                    if( (entity_type != MC_ENT_TYPE_ANY) && ((int)entity.GetEntityType() != entity_type) )
                    {
                        ret=MC_ASSET_KEY_INVALID_NAME;
                    }
                }
#else
                // TODO : how to implement
                //tempory return
                return MC_ASSET_KEY_VALID;
                assert(!"how to implement");
#endif
            }
        }
        else
        {
            ret=MC_ASSET_KEY_INVALID_SIZE;
        }
    }

    if(ret == MC_ASSET_KEY_VALID)
    {
        if(txid)
        {
            memcpy(txid,entity.GetTxID(),32);
        }
        ptr=entity.GetRef();
        if(entity.IsUnconfirmedGenesis())
        {
            ret=MC_ASSET_KEY_UNCONFIRMED_GENESIS;
        }
        if(asset_ref)
        {
            memcpy(asset_ref,ptr,MC_AST_ASSET_REF_SIZE);
        }
        if(name)
        {
            strcpy(name,entity.GetName());
        }
        if(multiple)
        {
            *multiple=entity.GetAssetMultiple();
        }
    }


    return ret;
}

void ParseEntityIdentifier(Value entity_identifier,mc_EntityDetails *entity,uint32_t entity_type)
{
    unsigned char buf[32];
    unsigned char buf_a[MC_AST_ASSET_REF_SIZE];
    unsigned char buf_n[MC_AST_ASSET_REF_SIZE];
    int ret;
    string entity_nameU;
    string entity_nameL;

    switch(entity_type)
    {
        case MC_ENT_TYPE_STREAM:
            entity_nameU="Stream";
            entity_nameL="stream";
            break;
        case MC_ENT_TYPE_ASSET:
            entity_nameU="Asset";
            entity_nameL="asset";
            break;
        case MC_ENT_TYPE_UPGRADE:
            entity_nameU="Upgrade";
            entity_nameL="upgrade";
            break;
        default:
            entity_nameU="Entity";
            entity_nameL="entity";
            break;
    }

    if (entity_identifier.type() != null_type && !entity_identifier.get_str().empty())
    {
        string str=entity_identifier.get_str();

        if(entity_type & MC_ENT_TYPE_STREAM)
        {
            if(AssetRefDecode(buf_a,str.c_str(),str.size()))
            {
                memset(buf_n,0,MC_AST_ASSET_REF_SIZE);
                if(memcmp(buf_a,buf_n,4) == 0)
                {
                    unsigned char *root_stream_name;
                    int root_stream_name_size;
                    // TODO : root_stream_name is from "rootstreamname" param
                    //root_stream_name=(unsigned char *)mc_gState->m_NetworkParams->GetParam("rootstreamname",&root_stream_name_size);
                    root_stream_name = (unsigned char*)("root");
                    if( (root_stream_name_size > 1) && (memcmp(buf_a,buf_n,MC_AST_ASSET_REF_SIZE) == 0) )
                    {
                        str=strprintf("%s",root_stream_name);
                    }
                    else
                    {
                        throw std::to_string(RPC_ENTITY_NOT_FOUND) + string("Stream with this stream reference not found: ") +str;
                    }
                }
            }
        }

        ret=ParseAssetKey(str.c_str(),buf,NULL,NULL,NULL,NULL,entity_type);
        switch(ret)
        {
            case MC_ASSET_KEY_INVALID_TXID:
                throw to_string(RPC_ENTITY_NOT_FOUND) + entity_nameU + string(" with this txid not found: ") + str;
                break;
            case MC_ASSET_KEY_INVALID_REF:
                throw to_string(RPC_ENTITY_NOT_FOUND) + entity_nameU + string(" with this reference not found: ")+str;
                break;
            case MC_ASSET_KEY_INVALID_NAME:
                throw to_string(RPC_ENTITY_NOT_FOUND) + entity_nameU+string(" with this name not found: ")+str;
                break;
            case MC_ASSET_KEY_INVALID_SIZE:
                throw to_string(RPC_INVALID_PARAMETER) + "Could not parse "+entity_nameL+" key: "+str;
                break;
        }
    }
    else
    {
        throw to_string(RPC_INVALID_PARAMETER) + "Invalid "+entity_nameL+" identifier";
    }

    if(entity)
    {
#if 0
        if(mc_gState->m_Assets->FindEntityByTxID(entity,buf))
        {
            if((entity_type & entity->GetEntityType()) == 0)
            {
                throw to_string(RPC_ENTITY_NOT_FOUND) + "Invalid "+entity_nameL+" identifier, not "+entity_nameL;
            }
        }
#else
        // TODO : how to implement
        assert(!"how to implement");
#endif
    }
}

bool ExtractDestinationScriptValid(const CScript& scriptPubKey, CTxDestination& addressRet)
{
    CScript::const_iterator pc1 = scriptPubKey.begin();
    unsigned char *ptr;
    int size;

    ptr=(unsigned char*)(&pc1[0]);
    size=scriptPubKey.end()-pc1;

    if( (size >= 25) && (ptr[0] == OP_DUP) )                                    // pay-to-pubkeyhash
    {
        addressRet = CKeyID(*(uint160*)(ptr+3));
        return true;
    }

    if( (size >= 23) && (ptr[0] == OP_HASH160) )                                // pay-to-scripthash
    {
        addressRet = CScriptID(*(uint160*)(ptr+2));
        return true;

    }

    if( size >= 35 )                                                            // pay-to-pubkey
    {
        if( (ptr[0] >= 33) && (ptr[0] <= 65) )
        {
            if(size >= 2+ptr[0])
            {
                CPubKey pubKey(ptr+1, ptr+1+ptr[0]);
                if (!pubKey.IsValid())
                    return false;

                addressRet = pubKey.GetID();
                return true;
            }
        }
    }

    return false;
}



bool sign(const CKey &privateKey, uint256 hash, int nHashType, CScript& scriptSigRet)
{
    vector<unsigned char> vchSig;
    if (!privateKey.Sign(hash, vchSig))    {
        throw "sign error";
    }
    vchSig.push_back((unsigned char)nHashType);
    scriptSigRet << vchSig;

    return true;
}

bool sign(const string &privateKey, uint256 hash, int nHashType, const IPrivateKeyHelper& helper, CScript& scriptSigRet)
{
    CBitcoinSecret vchSecret{helper};

    bool fGood = vchSecret.SetString(privateKey);

    if (!fGood)  {
        throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Invalid private key";
    }
    CKey key = vchSecret.GetKey();
    if (!key.IsValid()) {
        throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Private key outside allowed range";
    }

    return sign(key, hash, nHashType, scriptSigRet);
}

typedef vector<unsigned char> valtype;

// TODO : multi privatekeys
bool signN(const vector<valtype>& multisigdata, const string& privateKey, uint256 hash, int nHashType, const IPrivateKeyHelper& helper, CScript& scriptRet)
{
    bool signOk = false;

    int nSigned = 0;
    int nRequired = multisigdata.front()[0];

    for (unsigned int i = 1; i < multisigdata.size()-1 && (nSigned < nRequired); i++)    {
        const valtype& pubkey = multisigdata[i];
        CKeyID keyID = CPubKey(pubkey).GetID();
        // We are trying to use at least one address with send permission
        CBitcoinSecret vchSecret{helper};

        bool fGood = vchSecret.SetString(privateKey);
        if (!fGood)  {
            throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Invalid private key";
        }
        CKey key = vchSecret.GetKey();
        if (!key.IsValid()) {
            throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Private key outside allowed range";
        }

        if (key.GetPubKey().GetID() != keyID) {
            continue;
        }

        if (sign(key, hash, nHashType, scriptRet)) {
            ++nSigned;
        }
    }
                                                                                // As a result of looking for send permission we may can sign more than required
    signOk = nSigned>=nRequired;
    return signOk;
}

CKey keyFromPrivateKey(const string& privateKey, const IPrivateKeyHelper& helper)
{
    CBitcoinSecret vchSecret{helper};
    bool fGood = vchSecret.SetString(privateKey);
    if (!fGood)  {
        throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Invalid private key";
    }
    CKey key = vchSecret.GetKey();
    if (!key.IsValid()) {
        throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Private key outside allowed range";
    }
    return key;
}

bool solver(const string& privateKey, const IPrivateKeyHelper& helper, const CScript& scriptPubKey, uint256 hash, int nHashType,
            const string& unspentRedeemScript, CScript& scriptSigRet, txnouttype& whichTypeRet)
{
    vector<valtype> vSolutions;
    if (!TemplateSolver(scriptPubKey, whichTypeRet, vSolutions))
        return false;

    scriptSigRet.clear();

    EccAutoInitReleaseHandler eccScoper;

    CKeyID keyID;
    switch (whichTypeRet)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
        return false;
    case TX_PUBKEY: {
            keyID = CPubKey(vSolutions[0]).GetID();

            CKey keyFromPriv = keyFromPrivateKey(privateKey, helper);
            if (keyFromPriv.GetPubKey().GetID() != keyID) {
                return false;
            }

            return sign(keyFromPriv, hash, nHashType, scriptSigRet);
        }

    case TX_PUBKEYHASH: {
            keyID = CKeyID(uint160(vSolutions[0]));

            CKey keyFromPriv = keyFromPrivateKey(privateKey, helper);
            auto checkKeyID = keyFromPriv.GetPubKey().GetID();
            cout << "from script: " << HexStr(keyID) << endl;
            cout << "from private key: " << HexStr(checkKeyID) << endl;
            if (keyFromPriv.GetPubKey().GetID() != keyID) {
                return false;
            }

            if (!sign(keyFromPriv, hash, nHashType, scriptSigRet))
                return false;
            else
            {
                CPubKey vch = keyFromPriv.GetPubKey();
                scriptSigRet << ToByteVector(vch);
            }
            return true;
        }

    case TX_SCRIPTHASH: {
            //cout << "scripthash: " << HexStr(uint160(vSolutions[0])) << endl;
            //return keystore.GetCScript(uint160(vSolutions[0]), scriptSigRet);
            // from ./hdac-cli kcc listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', redeemScript
            // TODO : should check redeemScript can be from this
            // maybe HexStr(uint160(vSolutions[0])) is the ID for redeem script
            auto hexScriptSig = ParseHex(unspentRedeemScript);
            scriptSigRet = CScript(hexScriptSig.begin(), hexScriptSig.end());
        }
        return true;

    case TX_MULTISIG:
        scriptSigRet << OP_0; // workaround CHECKMULTISIG bug
        return signN(vSolutions, privateKey, hash, nHashType, helper, scriptSigRet);
    }
}

