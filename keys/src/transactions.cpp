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

// TODO : COIN is from nativecurrencymultiple param
static int64_t COIN = 100000000;
// TODO : MAX_MONEY is from "maximumperoutput"
static int64_t MAX_MONEY = 21000000 * COIN;

inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
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
    if(size == 64)
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

Value createrawsendfrom(const string &strAddr, const string &jsonAddrOrAmount,
                        const string &jsonAppendRawdata,
                        const IWalletAddrHelper& helper)
{
    vector<CTxDestination> fromaddresses;
    set<CTxDestination> thisFromAddresses;

    fromaddresses=ParseAddresses(strAddr, true, helper);

    if(fromaddresses.size() != 1)
    {
        throw "Single from-address should be specified";
    }

#if 0   // TODO : how to handle mine
    if( IsMine(*pwalletMain, fromaddresses[0]) == ISMINE_NO )
    {
        throw JSONRPCError(RPC_WALLET_ADDRESS_NOT_FOUND, "from-address is not found in this wallet");
    }
#endif


    for (const CTxDestination& fromaddress : fromaddresses) {
        thisFromAddresses.insert(fromaddress);
    }

    //Object sendTo = params[1].get_obj();
    Value addrOrAmount;
    if (read_string(jsonAddrOrAmount, addrOrAmount) == false)    {
        throw "json error : addr or amount is not valid";
    }
    Object sendTo = addrOrAmount.get_obj();

    vector <pair<CScript, CAmount> > vecSend;
    vecSend=ParseRawOutputMultiObject(sendTo,NULL);

    std::unique_ptr<mc_Buffer> assetOut(new mc_Buffer);
    std::unique_ptr<mc_Script> script(new mc_Script);
    //mc_gState->m_TmpAssetsOut->Clear();

    for(int i=0;i<(int)vecSend.size();i++)
    {
        //FindFollowOnsInScript(vecSend[i].first,mc_gState->m_TmpAssetsOut,mc_gState->m_TmpScript);
        FindFollowOnsInScript(vecSend[i].first,assetOut.get(),script.get());
    }

    mc_EntityDetails entity;
    entity.Zero();

    //if(mc_gState->m_TmpAssetsOut->GetCount())
    if(assetOut->GetCount())
    {
#if 0
        if(!mc_gState->m_Assets->FindEntityByFullRef(&entity,mc_gState->m_TmpAssetsOut->GetRow(0)))
        {
            throw "Follow-on script rejected - asset not found";
        }
#else
        // TODO : how to implement
        assert(!"how to implement");
#endif
    }

    mc_EntityDetails found_entity;

    Value appendRawdata;
    read_string(jsonAppendRawdata, appendRawdata);
    //if (params.size() > 2 && params[2].type() != null_type)    {
    if (appendRawdata.type() != null_type) {
        //for (const Value& data : params[2].get_array()) {
        for (const Value& data : appendRawdata.get_array()) {
            CScript scriptOpReturn=ParseRawMetadata(data,0x01FF,&entity,&found_entity);
#if 0
            if(found_entity.GetEntityType() == MC_ENT_TYPE_STREAM)
            {
                FindAddressesWithPublishPermission(fromaddresses,&found_entity);
            }
#endif
            vecSend.push_back(make_pair(scriptOpReturn, 0));
        }
    }

#if 0 // TODO : should implement
    string action="";
    string hex;
    Value signedTx;
    Value txid;
    bool sign_it=false;
    bool lock_it=false;
    bool send_it=false;
    if (params.size() > 3 && params[3].type() != null_type)
    {
        ParseRawAction(params[3].get_str(),lock_it,sign_it,send_it);
    }


    CReserveKey reservekey(pwalletMain);
    CAmount nFeeRequired;
    string strError;
    uint32_t flags=MC_CSF_ALLOW_NOT_SPENDABLE_P2SH | MC_CSF_ALLOW_SPENDABLE_P2SH | MC_CSF_ALLOW_NOT_SPENDABLE;

    if(!sign_it)
    {
        flags |= MC_CSF_ALLOWED_COINS_ARE_MINE;
    }

    if(vecSend.size() == 0)
    {
        throw "Either addresses object or data array should not be empty";
    }

    CWalletTx rawTx;

    EnsureWalletIsUnlocked();
    {
        LOCK (pwalletMain->cs_wallet_send);
        int eErrorCode;
        if(!CreateAssetGroupingTransaction(pwalletMain, vecSend, rawTx, reservekey, nFeeRequired, strError, NULL, &thisFromAddresses, 1, -1, -1, NULL, flags, &eErrorCode))
        {
            if(fDebug>0)LogPrintf("createrawsendfrom : %s\n", strError);
            throw JSONRPCError(eErrorCode, strError);
        }
    }

    hex=EncodeHexTx(rawTx);


    if(sign_it)
    {
        Array signrawtransaction_params;
        signrawtransaction_params.push_back(hex);
        signedTx=signrawtransaction(signrawtransaction_params,false);
    }
    if(lock_it)
    {
        for (const CTxIn& txin : rawTx.vin) {
            COutPoint outpt(txin.prevout.hash, txin.prevout.n);
            pwalletMain->LockCoin(outpt);
        }
    }
    if(send_it)
    {
        Array sendrawtransaction_params;
        for (const Pair& s : signedTx.get_obj())    {
            if(s.name_=="complete")
            {
                if(!s.value_.get_bool())
                {
                    throw JSONRPCError(RPC_TRANSACTION_ERROR, "Transaction was not signed properly");
                }
            }
            if(s.name_=="hex")
            {
                sendrawtransaction_params.push_back(s.value_.get_str());
            }
        }
        txid=sendrawtransaction(sendrawtransaction_params,false);
    }

    if(send_it)
    {
        return txid;
    }

    if(sign_it)
    {
        return signedTx;
    }

    return hex;
#endif
    return string();
}

