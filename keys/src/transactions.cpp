#include "transactions.h"
#include <json_spirit/json_spirit.h>
#include <script/standard.h>
#include <set>
#include "bitcoinaddress.h"
#include "key.h"
#include <json_spirit/json_spirit_reader_template.h>
#include <structs/amount.h>
#include "multisig.h"
#include <utils/utilsfront.h>
#include <protocol/hdacscript.h>
#include <entities/asset.h>
#include <utils/tinyformat.h>
#include <script/utilparse.h>
#include "rawmetadata.h"
#include <utils/define.h>
#include <primitives/transaction.h>
#include <primitives/interpreter.h>
#include "standard.h"
#include "bitcoinsecret.h"

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

class SampleWalletAddrHelper : public IWalletAddrHelper {
public:
    SampleWalletAddrHelper() { }

    const std::vector<unsigned char> pubkeyAddrPrefix() const override  {
        return ParseHex("003fd61c");
    }
    const std::vector<unsigned char> scriptAddrPrefix() const override  {
        return ParseHex("0571a3e6");
    }

    int32_t addrChecksumValue() const override {
        return parseHexToInt32Le("cb507245");
    }
};

CScript GetScriptForString(string source)
{
    vector<string> destinations;

    string tok;

    stringstream ss(source);
    while(getline(ss, tok, ','))
    {
        destinations.push_back(tok);
    }

    if(destinations.size() == 0)
    {
        throw runtime_error(" Address cannot be empty");
    }

    if(destinations.size() == 1)
    {
        // TODO : SampleWalletAddrHelper to external
        CBitcoinAddress address(destinations[0], SampleWalletAddrHelper());
#if 0   // TODO : how to handle pwalletMain
        if (pwalletMain && address.IsValid())
#else
        if (address.IsValid())
#endif
        {
            return GetScriptForDestination(address.Get());
        }
        else
        {
            if (IsHex(destinations[0]))
            {
                CPubKey vchPubKey(ParseHex(destinations[0]));
                if (!vchPubKey.IsFullyValid())
                    throw runtime_error(" Invalid public key: "+destinations[0]);
                return GetScriptForPubKey(vchPubKey);
            }
            else
            {
                throw runtime_error(" Invalid public key: "+destinations[0]);
            }
        }
    }

    //int required=atoi(destinations[0]);
    int required=stoi(destinations[0]);
    if( (required <= 0) || (required > 16) )
        throw runtime_error(" Invalid required for bare multisig: "+destinations[0]);

    if(required > (int)destinations.size()-1)
        throw runtime_error(" To few public keys");

    vector <CPubKey> vPubKeys;
    for(int i=1;i<(int)destinations.size();i++)
    {
        CPubKey vchPubKey(ParseHex(destinations[i]));
        if (!vchPubKey.IsFullyValid())
            throw runtime_error(" Invalid public key: "+destinations[i]);
        vPubKeys.push_back(vchPubKey);
    }

    return GetScriptForMultisig(required,vPubKeys);
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

int ParseAssetKeyToFullAssetRef(const char* asset_key,unsigned char *full_asset_ref,int *multiple,int *type,int entity_type)
{
    int ret;
    unsigned char txid[MC_ENT_KEY_SIZE];
    ret=ParseAssetKey(asset_key,txid,NULL,NULL,multiple,type,entity_type);
    if(ret == MC_ASSET_KEY_UNCONFIRMED_GENESIS)
    {
        ret=0;
    }
    memcpy(full_asset_ref+MC_AST_SHORT_TXID_OFFSET,txid+MC_AST_SHORT_TXID_OFFSET,MC_AST_SHORT_TXID_SIZE);

    mc_SetABRefType(full_asset_ref,MC_AST_ASSET_REF_TYPE_SHORT_TXID);

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

// TODO : where to locate
string ParseRawOutputObject(Value param,CAmount& nAmount,mc_Script *lpScript, int *required,int *eErrorCode)
{
    // TODO : maxstdelementsize from params
    static unsigned int MAX_SCRIPT_ELEMENT_SIZE = 520;

    string strError="";
    unsigned char buf[MC_AST_ASSET_FULLREF_BUF_SIZE];
    //mc_Buffer *lpBuffer=mc_gState->m_TmpBuffers->m_RpcABNoMapBuffer1;
    //lpBuffer->Clear();
    std::unique_ptr<mc_Buffer> buffer(new mc_Buffer);
    //mc_Buffer *lpFollowonBuffer=mc_gState->m_TmpBuffers->m_RpcABNoMapBuffer2;
    //lpFollowonBuffer->Clear();
    std::unique_ptr<mc_Buffer> followonBuffer(new mc_Buffer);

    int assets_per_opdrop=(MAX_SCRIPT_ELEMENT_SIZE-4)/(m_AssetRefSize+MC_AST_ASSET_QUANTITY_SIZE);
    int32_t verify_level=-1;
    int asset_error=0;
    int multiple;
    int64_t max_block=0xffffffff;
    string asset_name;
    string type_string;
    nAmount=0;

    if(eErrorCode)
    {
        *eErrorCode=RPC_INVALID_PARAMETER;
    }

    memset(buf,0,MC_AST_ASSET_FULLREF_BUF_SIZE);

    //BOOST_FOREACH(const Pair& a, param.get_obj())
    for (const Pair& a : param.get_obj())    {
        if(a.value_.type() == obj_type)    {
            bool parsed=false;

            if(!parsed && (a.name_ == "hidden_verify_level"))            {
                //BOOST_FOREACH(const Pair& d, a.value_.get_obj())                {
                for (const Pair& d : a.value_.get_obj()) {
                    bool field_parsed=false;
                    if(!field_parsed && (d.name_ == "value"))
                    {
                        verify_level=d.value_.get_int();
                        field_parsed=true;
                    }
                    if(!field_parsed)
                    {
                        strError=string("Invalid field for object ") + a.name_ + string(": ") + d.name_;
                        goto exitlbl;
                    }
                }
                parsed=true;
            }

            if(!parsed && (a.name_ == "issue"))
            {
                int64_t quantity=-1;
                //BOOST_FOREACH(const Pair& d, a.value_.get_obj()){
                for (const Pair& d : a.value_.get_obj())    {
                    bool field_parsed=false;
                    if(!field_parsed && (d.name_ == "raw")) {
                        if (d.value_.type() != null_type)   {
                            quantity=d.value_.get_int64();
                            if(quantity<0)  {
                                strError=string("Negative value for issue raw qty");
                                goto exitlbl;
                            }
                        }
                        else    {
                            strError=string("Invalid value for issue raw qty");
                            goto exitlbl;

                        }
                        field_parsed=true;
                    }
                    if(!field_parsed)   {
                        strError=string("Invalid field for object ") + a.name_ + string(": ") + d.name_;
                        goto exitlbl;
                    }
                }
                if(quantity < 0)    {
                    strError=string("Issue raw qty not specified");
                    goto exitlbl;
                }
                lpScript->SetAssetGenesis(quantity);
                parsed=true;
            }

            if(!parsed && (a.name_ == "issuemore")) {
                int64_t quantity=-1;
                asset_name="";
                //BOOST_FOREACH(const Pair& d, a.value_.get_obj())  {
                for (const Pair& d : a.value_.get_obj()) {
                    bool field_parsed=false;
                    if(!field_parsed && (d.name_ == "raw")) {
                        if (d.value_.type() != null_type)   {
                            quantity=d.value_.get_int64();
                            if(quantity<0)  {
                                strError=string("Negative value for issuemore raw qty");
                                goto exitlbl;
                            }
                        }
                        else    {
                            strError=string("Invalid value for issuemore raw qty");
                            goto exitlbl;

                        }
                        field_parsed=true;
                    }
                    if(!field_parsed && (d.name_ == "asset"))   {
                        if(d.value_.type() != null_type && !d.value_.get_str().empty()) {
                            asset_name=d.value_.get_str();
                        }
                        if(asset_name.size())   {
                            asset_error=ParseAssetKeyToFullAssetRef(asset_name.c_str(),buf,&multiple,NULL, MC_ENT_TYPE_ASSET);
                            if(asset_error) {
                                goto exitlbl;
                            }
                            field_parsed=true;
                        }
                    }
                    if(!field_parsed)   {
                        strError=string("Invalid field for object ") + a.name_ + string(": ") + d.name_;
                        goto exitlbl;
                    }
                }
                if(asset_name.size() == 0)  {
                    strError=string("Issuemore asset not specified");
                    goto exitlbl;
                }
                if(quantity < 0)    {
                    strError=string("Issuemore raw qty not specified");
                    goto exitlbl;
                }
                //if(lpFollowonBuffer->GetCount())    {
                if(followonBuffer->GetCount())    {
                    if(verify_level & 0x0008)   {
                        //if(memcmp(buf,lpFollowonBuffer->GetRow(0),MC_AST_ASSET_QUANTITY_OFFSET))    {
                        if(memcmp(buf,followonBuffer->GetRow(0),MC_AST_ASSET_QUANTITY_OFFSET))    {
                            strError=string("Issuemore for different assets");
                            goto exitlbl;
                        }
                    }
                    //lpFollowonBuffer->Clear();
                    followonBuffer->Clear();
                }
                mc_SetABQuantity(buf,quantity);
                //lpFollowonBuffer->Add(buf);
                followonBuffer->Add(buf);
                //lpScript->SetAssetQuantities(lpFollowonBuffer,MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON);
                lpScript->SetAssetQuantities(followonBuffer.get(),MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON);
                parsed=true;
            }

            if(!parsed && (a.name_ == "permissions"))   {
                uint32_t type,from,to,timestamp;
                int64_t v;
                mc_EntityDetails entity;
                entity.Zero();

                type_string="";
                type=0;
                from=0;
                to=4294967295U;
                timestamp=mc_TimeNowAsUInt();

                //BOOST_FOREACH(const Pair& d, a.value_.get_obj())  {
                for (const Pair& d : a.value_.get_obj())    {
                    bool field_parsed=false;

                    if(!field_parsed && (d.name_ == "for")) {
                        entity.Zero();
                        if(d.value_.type() != null_type && !d.value_.get_str().empty()) {
                            ParseEntityIdentifier(d.value_,&entity, MC_ENT_TYPE_ANY);
                        }
                        field_parsed=true;
                    }
                    if(!field_parsed && (d.name_ == "type"))    {
                        if(d.value_.type() == str_type && !d.value_.get_str().empty())  {
                            type_string=d.value_.get_str();
                        }
                        else    {
                            strError=string("Invalid value for permission type");
                            goto exitlbl;

                        }
                        field_parsed=true;
                    }
                    if(!field_parsed && (d.name_ == "startblock"))  {
                        if (d.value_.type() != null_type)   {

                            v=d.value_.get_int64();
                            if(v<0) {
                                strError=string("Negative value for permissions startblock");
                                goto exitlbl;
                            }
                            if(v>max_block) {
                                strError=string("Invalid value for permissions endblock");
                                goto exitlbl;
                            }
                            from=v;
                        }
                        else    {
                            strError=string("Invalid value for permissions startblock");
                            goto exitlbl;

                        }
                        field_parsed=true;
                    }
                    if(!field_parsed && (d.name_ == "endblock"))    {
                        if (d.value_.type() != null_type)   {
                            v=d.value_.get_int64();
                            if(v<0) {
                                strError=string("Negative value for permissions endblock");
                                goto exitlbl;
                            }
                            if(v>max_block) {
                                strError=string("Invalid value for permissions endblock");
                                goto exitlbl;
                            }
                            to=v;
                        }
                        else    {
                            strError=string("Invalid value for permissions endblock");
                            goto exitlbl;

                        }
                        field_parsed=true;
                    }
                    if(!field_parsed && (d.name_ == "timestamp"))   {
                        if (d.value_.type() != null_type)   {
                            timestamp=(uint32_t)d.value_.get_uint64();
                        }
                        else    {
                            strError=string("Invalid value for permissions timestamp");
                            goto exitlbl;

                        }
                        field_parsed=true;
                    }
                    if(!field_parsed)   {
                        strError=string("Invalid field for object ") + a.name_ + string(": ") + d.name_;
                        goto exitlbl;
                    }
                }

                if(type_string.size())  {
#if 0
                    type=mc_gState->m_Permissions->GetPermissionType(type_string.c_str(),entity.GetEntityType());
                    if(entity.GetEntityType() == MC_ENT_TYPE_NONE)  {
                        if(required)    {
                            *required |= type;
                        }
                    }
                    if(type == 0)   {
                        strError=string("Invalid value for permission type: ") + type_string;
                        goto exitlbl;
                    }
#else
                    // TODO : how to implement
                    assert(!"how to implement");
#endif
                }

                if(type == 0)   {
                    strError=string("Permission type not specified");
                    goto exitlbl;
                }

                if(entity.GetEntityType())  {
                    lpScript->SetEntity(entity.GetTxID()+MC_AST_SHORT_TXID_OFFSET);
                }
                lpScript->SetPermission(type,from,to,timestamp);
                parsed=true;
            }

            if(!parsed) {
                strError=string("Invalid object: ") + a.name_;
                goto exitlbl;
            }
        }
        else    {
            if(a.name_.size())  {
                asset_name=a.name_;
                asset_error=ParseAssetKeyToFullAssetRef(asset_name.c_str(),buf,&multiple,NULL, MC_ENT_TYPE_ASSET);
                if(asset_error) {
                    goto exitlbl;
                }
                int64_t quantity = (int64_t)(a.value_.get_real() * multiple + 0.499999);
                if(verify_level & 0x0010)   {
                    if(quantity < 0)    {
                        strError=string("Negative asset quantity");
                        goto exitlbl;
                    }
                }
                mc_SetABQuantity(buf,quantity);
                //lpBuffer->Add(buf);
                buffer->Add(buf);
                if(verify_level & 0x0001)   {
                    //if(lpBuffer->GetCount() >= assets_per_opdrop)   {
                    if(buffer->GetCount() >= assets_per_opdrop)   {
                        lpScript->SetAssetQuantities(buffer.get(),MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER);
                        //lpBuffer->Clear();
                        buffer->Clear();
                    }
                }
            }
            else    {
                nAmount += AmountFromValue(a.value_);
            }
        }
    }

    // TODO : fRequireStandard is following
    //fRequireStandard = (mc_gState->m_NetworkParams->GetInt64Param("onlyacceptstdtxs") != 0);
    //fRequireStandard=GetBoolArg("-requirestandard", fRequireStandard);
    static bool fRequireStandard = true;
    //if(lpBuffer->GetCount())    {
    if(buffer->GetCount())    {
        //if(Params().RequireStandard())  {
        if (fRequireStandard) {
            if(verify_level & 0x0002)   {
                //if(lpBuffer->GetCount() > assets_per_opdrop)    {
                if(buffer->GetCount() > assets_per_opdrop)    {
                    strError=string("Too many assets in one group");
                    goto exitlbl;
                }
            }
        }
        //lpScript->SetAssetQuantities(lpBuffer,MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER);
        lpScript->SetAssetQuantities(buffer.get(),MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER);
        //lpBuffer->Clear();
        buffer->Clear();
    }

    // TODO : should get MCP_STD_OP_DROP_COUNT from mc_OneHdacParam
    static int MCP_STD_OP_DROP_COUNT = 5;
    if(verify_level & 0x0004)    {
        //if(Params().RequireStandard())  {
        if (fRequireStandard) {
            if(lpScript->GetNumElements() > MCP_STD_OP_DROP_COUNT)  {
                strError=string("Too many objects in output");
                goto exitlbl;
            }
        }
    }

exitlbl:

    switch(asset_error)    {
        case MC_ASSET_KEY_INVALID_TXID:
            if(eErrorCode)  {
                *eErrorCode=RPC_ENTITY_NOT_FOUND;
            }
            strError=string("Issue transaction with this txid not found: ")+asset_name;
            break;
        case MC_ASSET_KEY_INVALID_REF:
            if(eErrorCode)  {
                *eErrorCode=RPC_ENTITY_NOT_FOUND;
            }
            strError=string("Issue transaction with this asset reference not found: ")+asset_name;
            break;
        case MC_ASSET_KEY_INVALID_NAME:
            if(eErrorCode)  {
                *eErrorCode=RPC_ENTITY_NOT_FOUND;
            }
            strError=string("Issue transaction with this name not found: ")+asset_name;
            break;
        case MC_ASSET_KEY_INVALID_SIZE:
            strError=string("Could not parse asset key: ")+asset_name;
            break;
        case MC_ASSET_KEY_UNCONFIRMED_GENESIS:
            if(eErrorCode)  {
                *eErrorCode=RPC_UNCONFIRMED_ENTITY;
            }
            strError=string("Unconfirmed asset: ")+asset_name;
            break;
    }

    return strError;

}

string ParseRawOutputObject(Value param,CAmount& nAmount,mc_Script *lpScript,int *eErrorCode)
{
    return ParseRawOutputObject(param,nAmount,lpScript,NULL,eErrorCode);
}

vector <pair<CScript, CAmount> > ParseRawOutputMultiObject(Object sendTo,int *required)
{
    vector <pair<CScript, CAmount> > vecSend;

    set<CBitcoinAddress> setAddress;
    for (const Pair& s : sendTo)    {
        CScript scriptPubKey = GetScriptForString(s.name_);

        CAmount nAmount;

        if (s.value_.type() != obj_type)
        {
            nAmount = AmountFromValue(s.value_);
        }
        else
        {
            //mc_Script *lpScript=mc_gState->m_TmpBuffers->m_RpcScript4;
            //lpScript->Clear();

            std::unique_ptr<mc_Script> script(new mc_Script);


            nAmount=0;
            int eErrorCode;

            string strError=ParseRawOutputObject(s.value_,nAmount, script.get(), required, &eErrorCode);
            if(strError.size())
            {
                throw std::to_string(eErrorCode) + ": " + strError;
            }

            size_t elem_size;
            const unsigned char *elem;
            //for(int element=0;element < lpScript->GetNumElements();element++)
            for(int element=0;element < script->GetNumElements();element++)
            {
                //elem = lpScript->GetData(element,&elem_size);
                elem = script->GetData(element,&elem_size);
                if(elem)
                {
                    scriptPubKey << vector<unsigned char>(elem, elem + elem_size) << OP_DROP;
                }
                else
                    //throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid script");
                    throw to_string(RPC_INTERNAL_ERROR) + "Invalid script";
            }

        }
        vecSend.push_back(make_pair(scriptPubKey, nAmount));
    }

    return vecSend;
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


class SamplePrivateKeyHelper : public IPrivateKeyHelper {
public:
    SamplePrivateKeyHelper() { }

    const std::vector<unsigned char> privkeyPrefix() const override {
        return ParseHex("8075fa23");
    }

    int32_t addrChecksumValue() const override {
        return parseHexToInt32Le("cb507245");
    }
};

class EccAutoInitReleaseHandler
{
public:
    EccAutoInitReleaseHandler() {
        ECC_Start();
        verifyHandle.reset(new ECCVerifyHandle);
        if(!ECC_InitSanityCheck()) {
            cerr << "Elliptic curve cryptography sanity check failure. Aborting." << endl;
            //InitError("Elliptic curve cryptography sanity check failure. Aborting.");
            verifyHandle.reset();
            ECC_Stop();
            return;
        }
    }

    ~EccAutoInitReleaseHandler() {
        ECC_Stop();
    }

private:
    unique_ptr<ECCVerifyHandle> verifyHandle;
};

Value createMultisigStreamTx(const string &strAddr, const string& streamName,
                             const string& streamKey, const string& streamItem,
                             const string& createTxid,
                             const string& unspentScriptPubKey, const string& unspentTxid, uint32_t unspentVOut,
                             const string& unspentRedeemScript, const string& privateKey)
{
    EccAutoInitReleaseHandler eccScoper;

    mc_EntityDetails found_entity;

    // from CScript scriptOpReturn=ParseRawMetadata(data,0x01FF,&entity,&found_entity);
    vector <pair<CScript, CAmount> > vecSend;
    CScript scriptOpReturn;
    const unsigned char *script;
    std::unique_ptr<mc_Script> detailsScript(new mc_Script);
    detailsScript->Clear();
    // from liststreams ${streamName}
    auto hexTxid = ParseHex(createTxid);

    std::reverse(hexTxid.begin(), hexTxid.end());
    //detailsScript->SetEntity(entity.GetTxID()+MC_AST_SHORT_TXID_OFFSET);
    detailsScript->SetEntity(hexTxid.data()+MC_AST_SHORT_TXID_OFFSET);
    size_t bytes;
    script = detailsScript->GetData(0,&bytes);
    scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP;

    vector<unsigned char> vKey(streamKey.begin(), streamKey.end());
    vector<unsigned char> vValue(streamItem.begin(), streamItem.end());

    detailsScript->Clear();
    if(detailsScript->SetItemKey(&vKey[0],vKey.size()) == MC_ERR_NOERROR)
    {
        script = detailsScript->GetData(0,&bytes);
        scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP;
    }

    scriptOpReturn << OP_RETURN << vValue;
    vecSend.push_back(make_pair(scriptOpReturn, 0));


    string hex;

    CMutableTransaction txNew;
    for (const auto& s : vecSend)   {
        CTxOut txout(s.second, s.first);
        txNew.vout.push_back(txout);
    }

    //string strPubkeyScript = "a9143e45d3a48882576ad5900978303705e1a6000305871473706b700600000000000000ffffffff52ff095c75";
    //string strPubkeyScript = "a9143e45d3a48882576ad5900978303705e1a6000305871473706b658f70622f63195d4844d12f6f8c9eb5a0751473706b700800000000000000ffffffff24fc095c75";     
    // from ./hdac-cli kcc listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', scriptPubKey

    auto pubkeyScript = ParseHex(unspentScriptPubKey);

    CTxOut txOut1(0, CScript(pubkeyScript.begin(), pubkeyScript.end()));
    const CScript& script1 = txOut1.scriptPubKey;

    // TODO : check, where from destination
    CTxDestination addressRet;
    ExtractDestinationScriptValid(script1, addressRet);

    CScript scriptChange=GetScriptForDestination(addressRet);
    CTxOut txout2(0, scriptChange);
    txNew.vout.push_back(txout2);

    //txNew.vin.push_back(CTxIn(uint256("812b4f1732cfacde79511b2d49a851a05ead33bb4e79926a7351946ae49665bc"), 0/*out.i*/));
    //txNew.vin.push_back(CTxIn(uint256("21386e1f4e1d9cfe7c858f7d23486e1a7da4d9c86c3a19e73a4cad4fdba8a4dc"), 0/*out.i*/));
    // from ./hdac-cli kcc listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', txid

    txNew.vin.push_back(CTxIn(uint256(unspentTxid), unspentVOut));

    hex=EncodeHexTx(txNew);
    cout << "before sign, TxHex: " << hex << endl;


    int nIn = 0;
    int nHashType = SIGHASH_ALL;

    uint256 hash = SignatureHash(script1, txNew, nIn, nHashType);
    cout << "hash: " << HexStr(hash) << endl;

    CTxIn& txin = txNew.vin[nIn];

    //if (!Solver(keystore, script1, hash, nHashType, txin.scriptSig, whichType))
    //    return false;

    typedef vector<unsigned char> valtype;
    vector<valtype> vSolutions;
    txnouttype whichType;
    if (!TemplateSolver(script1, whichType, vSolutions))
        return false;

    CScript& scriptSigRet = txin.scriptSig;
    scriptSigRet.clear();

    CKeyID keyID;
    switch (whichType)
    {
#if 0
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
        return false;
    case TX_PUBKEY:
        keyID = CPubKey(vSolutions[0]).GetID();
        return Sign1(keyID, keystore, hash, nHashType, scriptSigRet);
    case TX_PUBKEYHASH:
        keyID = CKeyID(uint160(vSolutions[0]));
        if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
            return false;
        else
        {
            CPubKey vch;
            keystore.GetPubKey(keyID, vch);
            scriptSigRet << ToByteVector(vch);
        }
        return true;
#endif
    case TX_SCRIPTHASH: {
        cout << "scripthash: " << HexStr(uint160(vSolutions[0])) << endl;
        //return keystore.GetCScript(uint160(vSolutions[0]), scriptSigRet);
        // from ./hdac-cli kcc listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', redeemScript
        // TODO : should check redeemScript can be from this
#if 0
        auto hexScriptSig = ParseHex("5221027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b21038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd52ae");
#else
        auto hexScriptSig = ParseHex(unspentRedeemScript);
#endif
        scriptSigRet = CScript(hexScriptSig.begin(), hexScriptSig.end());
        }
        break;

    case TX_MULTISIG:
        scriptSigRet << OP_0; // workaround CHECKMULTISIG bug
        //return (SignN(vSolutions, keystore, hash, nHashType, scriptSigRet));
        //bool signOk = SignN(vSolutions, keystore, hash, nHashType, scriptSigRet);
        bool signOk = false;
        {
            int nSigned = 0;
            int nRequired = vSolutions.front()[0];

            for (unsigned int i = 1; i < vSolutions.size()-1 && (nSigned < nRequired); i++)
            {
                const valtype& pubkey = vSolutions[i];
                CKeyID keyID = CPubKey(pubkey).GetID();
                                                                                        // We are trying to use at least one address with send permission
                //if (Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
                //    ++nSigned;
                {


                    CBitcoinSecret vchSecret{SamplePrivateKeyHelper{}};

#if 0
                    string strPrivKey("VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp");
                    //bool fGood = vchSecret.SetString(string("VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp"));
                    bool fGood = vchSecret.SetString(strPrivKey);
#else
                    bool fGood = vchSecret.SetString(privateKey);
#endif

                    if (!fGood)  {
                        throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Invalid private key";
                    }
                    CKey key = vchSecret.GetKey();
                    if (!key.IsValid()) {
                        throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Private key outside allowed range";
                    }

                    vector<unsigned char> vchSig;
                    if (!key.Sign(hash, vchSig))    {
                        throw "sign error";
                    }
                    vchSig.push_back((unsigned char)nHashType);
                    scriptSigRet << vchSig;

                    ++nSigned;
                }
            }
                                                                                        // As a result of looking for send permission we may can sign more than required
            signOk = nSigned>=nRequired;
        }
    }

    if (whichType == TX_SCRIPTHASH) {
        CScript subscript = txin.scriptSig;
        uint256 hash2 = SignatureHash(subscript, txNew, nIn, nHashType);
        cout << "hash2: " << HexStr(hash2) << endl;
        {
            CTxIn& txin = txNew.vin[nIn];

            //if (!Solver(keystore, script1, hash, nHashType, txin.scriptSig, whichType))
            //    return false;

            typedef vector<unsigned char> valtype;
            vector<valtype> vSolutions;
            txnouttype subType;
            if (!TemplateSolver(subscript, subType, vSolutions))
                return false;

            CScript& scriptSigRet = txin.scriptSig;
            scriptSigRet.clear();

            CKeyID keyID;
            switch (subType)
            {
        #if 0
            case TX_NONSTANDARD:
            case TX_NULL_DATA:
                return false;
            case TX_PUBKEY:
                keyID = CPubKey(vSolutions[0]).GetID();
                return Sign1(keyID, keystore, hash, nHashType, scriptSigRet);
            case TX_PUBKEYHASH:
                keyID = CKeyID(uint160(vSolutions[0]));
                if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
                    return false;
                else
                {
                    CPubKey vch;
                    keystore.GetPubKey(keyID, vch);
                    scriptSigRet << ToByteVector(vch);
                }
                return true;
        #endif
            case TX_SCRIPTHASH: {
                cout << "scripthash: " << HexStr(uint160(vSolutions[0])) << endl;
                //return keystore.GetCScript(uint160(vSolutions[0]), scriptSigRet);
                // from ./hdac-cli kcc listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', redeemScript
#if 0
                scriptSigRet = CScript(ParseHex("5221027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b21038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd52ae"));
#else
                auto hexScriptSig = ParseHex(unspentRedeemScript);
                scriptSigRet = CScript(hexScriptSig.begin(), hexScriptSig.end());
#endif
            }
                break;

            case TX_MULTISIG:
                scriptSigRet << OP_0; // workaround CHECKMULTISIG bug
                //return (SignN(vSolutions, keystore, hash, nHashType, scriptSigRet));
                //bool signOk = SignN(vSolutions, keystore, hash, nHashType, scriptSigRet);
                bool signOk = false;
                {
                    int nSigned = 0;
                    int nRequired = vSolutions.front()[0];

                    for (unsigned int i = 1; i < vSolutions.size()-1 && (nSigned < nRequired); i++)
                    {
                        const valtype& pubkey = vSolutions[i];
                        CKeyID keyID = CPubKey(pubkey).GetID();
                                                                                                // We are trying to use at least one address with send permission
                        //if (Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
                        //    ++nSigned;
                        {
                            CBitcoinSecret vchSecret{SamplePrivateKeyHelper{}};

#if 0
                            string strPrivKey("VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp");

                            //bool fGood = vchSecret.SetString(string("VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp"));
                            bool fGood = vchSecret.SetString(strPrivKey);
#else
                            bool fGood = vchSecret.SetString(privateKey);
#endif

                            if (!fGood)  {
                                throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Invalid private key";
                            }
                            CKey key = vchSecret.GetKey();
                            if (!key.IsValid()) {
                                throw to_string(RPC_INVALID_ADDRESS_OR_KEY) + ": " + "Private key outside allowed range";
                            }

                            cout << "from private : " << key.GetPubKey().GetID().ToString() << endl;
                            cout << "from pubkey : " << keyID.ToString() << endl << endl;
                            if (key.GetPubKey().GetID() != keyID) {
                                continue;
                            }

                            vector<unsigned char> vchSig;
                            if (!key.Sign(hash2, vchSig))    {
                                throw "sign error";
                            }

                            vchSig.push_back((unsigned char)nHashType);
                            scriptSigRet << vchSig;

                            ++nSigned;
                        }
                    }
                                                                                                // As a result of looking for send permission we may can sign more than required
                    signOk = nSigned>=nRequired;
                }
            }
        }
        txin.scriptSig << static_cast<valtype>(subscript);
    }
    hex=EncodeHexTx(txNew);
    cout << "after sign, TxHex: " << hex << endl;


    return hex;
}
