#include "rawmetadata.h"
#include <entities/asset.h>
#include <utils/define.h>
#include <utils/utilstrencodings.h>
#include "transactions.h"
#include <utils/utility.h>
// TODO : remove the dependency with rpc
#include <rpc/rpcprotocol.h>

using namespace json_spirit;
using namespace std;

CScript ParseRawMetadata(Value param,uint32_t allowed_objects,mc_EntityDetails *given_entity,mc_EntityDetails *found_entity)
{
// codes for allowed_objects fields
// 0x0001 - create
// 0x0002 - publish
// 0x0004 - issue
// 0x0008 - follow-on
// 0x0010 - pure details
// 0x0020 - approval
// 0x0040 - create upgrade
// 0x0100 - encode empty hex
// 0x0200 - cache input script

    CScript scriptOpReturn=CScript();
    if(found_entity)
    {
        found_entity->Zero();
    }

    if(param.type() == obj_type)
    {
        mc_Script *lpDetailsScript;
        mc_Script *lpDetails;
        lpDetailsScript=new mc_Script;
        lpDetails=new mc_Script;
        lpDetails->AddElement();
        int format=0;
        string entity_name="";
        int new_type=0;
        int multiple=1;
        int is_open=0;
        bool multiple_is_set=false;
        bool open_is_set=false;
        string strError="";
        mc_EntityDetails entity;
        vector<unsigned char> vKey;
        vector<unsigned char> vValue;
        int protocol_version=-1;
        uint32_t startblock=0;
        int approve=-1;
        bool startblock_is_set=false;
//        bool key_is_set=false;
//        bool value_is_set=false;


        format=0;
        entity.Zero();
        //BOOST_FOREACH(const Pair& d, param.get_obj())
        for (const Pair& d : param.get_obj())
        {
            if(d.name_ == "name")
            {
                format |= 1;
            }
            if(d.name_ == "multiple")
            {
                format |= 2;
            }
        }

        //BOOST_FOREACH(const Pair& d, param.get_obj()) {
        for(const Pair& d : param.get_obj()) {
            bool parsed=false;

            if(d.name_ == "inputcache")
            {
                new_type=-3;
                if((allowed_objects & 0x0200) == 0 )
                {
                    strError=string("Keyword not allowed in this API");
                }
                else
                {
                    if(d.value_.type() != array_type)
                    {
                        strError=string("Array should be specified for inputcache");
                    }
                    else
                    {
                        int cs_offset,cs_vin,cs_size;
                        string cs_script="";
                        Array csa=d.value_.get_array();
                        lpDetails->Clear();
                        lpDetails->SetCachedScript(0,&cs_offset,-1,NULL,-1);
                        for(int csi=0;csi<(int)csa.size();csi++)
                        {
                            if(strError.size() == 0)
                            {
                                if(csa[csi].type() != obj_type)
                                {
                                    strError=string("Elements of inputcache should be objects");
                                }
                                cs_vin=-1;
                                cs_size=-1;
                                //BOOST_FOREACH(const Pair& csf, csa[csi].get_obj())    {
                                for (const Pair& csf : csa[csi].get_obj())  {
                                    bool cs_parsed=false;
                                    if(csf.name_ == "vin")
                                    {
                                        cs_parsed=true;
                                        if(csf.value_.type() != int_type)
                                        {
                                            strError=string("vin should be integer");
                                        }
                                        else
                                        {
                                            cs_vin=csf.value_.get_int();
                                        }
                                    }
                                    if(csf.name_ == "scriptPubKey")
                                    {
                                        cs_parsed=true;
                                        if(csf.value_.type() != str_type)
                                        {
                                            strError=string("scriptPubKey should be string");
                                        }
                                        else
                                        {
                                            cs_script=csf.value_.get_str();
                                            cs_size=cs_script.size()/2;
                                        }
                                    }
                                    if(!cs_parsed)
                                    {
                                        strError=string("Invalid field: ") + csf.name_;
                                    }
                                }
                                if(strError.size() == 0)
                                {
                                    if(cs_vin<0)
                                    {
                                        strError=string("Missing vin field");
                                    }
                                }
                                if(strError.size() == 0)
                                {
                                    if(cs_size<0)
                                    {
                                        strError=string("Missing scriptPubKey field");
                                    }
                                }
                                if(strError.size() == 0)
                                {
                                    bool fIsHex;
                                    vector<unsigned char> dataData(ParseHex(cs_script.c_str(),fIsHex));
                                    if(!fIsHex)
                                    {
                                        strError=string("scriptPubKey should be hexadecimal string");
                                    }
                                    else
                                    {
                                        lpDetails->SetCachedScript(cs_offset,&cs_offset,cs_vin,&dataData[0],cs_size);
                                    }
                                }
                            }
                        }
                    }
                }
                parsed=true;
            }

            if(d.name_ == "create")
            {
                if(new_type != 0)
                {
                    strError=string("Only one of the following keywords can appear in the object: create, update, for");
                }
                if(d.value_.type() != null_type && !d.value_.get_str().empty())
                {
                    if(d.value_.get_str() == "stream")
                    {
                        if((allowed_objects & 0x0001) == 0)
                        {
                            strError=string("Keyword not allowed in this API");
                        }
                        new_type=MC_ENT_TYPE_STREAM;
                    }
                    else
                    {
                        if(d.value_.get_str() == "asset")
                        {
                            if((allowed_objects & 0x0004) == 0)
                            {
                                strError=string("Keyword not allowed in this API");
                            }
                            new_type=MC_ENT_TYPE_ASSET;
                        }
                        else
                        {
                            if(d.value_.get_str() == "upgrade")
                            {
                                if((allowed_objects & 0x0040) == 0)
                                {
                                    strError=string("Keyword not allowed in this API");
                                }
                                new_type=MC_ENT_TYPE_UPGRADE;
                            }
                            else
                            {
                                strError=string("Invalid new entity type");
                            }
                        }
                    }
                }
                else
                {
                    strError=string("Invalid new entity type");
                }
                parsed=true;
            }

            if(d.name_ == "for")
            {
                if(new_type != 0)
                {
                    strError=string("Only one of the following keywords can appear in the object: create, update, for");
                }
                if(d.value_.type() != null_type && !d.value_.get_str().empty())
                {
                    ParseEntityIdentifier(d.value_,&entity, MC_ENT_TYPE_ANY);
                    if(found_entity)
                    {
                        memcpy(found_entity,&entity,sizeof(mc_EntityDetails));
                    }
                }
                if(entity.GetEntityType() == MC_ENT_TYPE_STREAM)
                {
                    new_type=-1;
                    if((allowed_objects & 0x0002) == 0)
                    {
                        strError=string("Keyword not allowed in this API");
                    }
                }
                else
                {
                    if(entity.GetEntityType() == MC_ENT_TYPE_UPGRADE)
                    {
                        new_type=-5;
                        if((allowed_objects & 0x0020) == 0)
                        {
                            strError=string("Keyword not allowed in this API");
                        }
                    }
                    else
                    {
                        strError=string("Entity with this identifier not found");
                    }
                }
                parsed=true;
            }

            if(d.name_ == "update")
            {
                if(new_type != 0)
                {
                    strError=string("Only one of the following keywords can appear in the object: create, update, for");
                }
                if(d.value_.type() != null_type && !d.value_.get_str().empty())
                {
                    ParseEntityIdentifier(d.value_,&entity, MC_ENT_TYPE_ASSET);
                    if(found_entity)
                    {
                        memcpy(found_entity,&entity,sizeof(mc_EntityDetails));
                    }
                }
                new_type=-2;
                if((allowed_objects & 0x0010) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                else
                {
                    if(entity.GetEntityType() != MC_ENT_TYPE_ASSET)
                    {
                        strError=string("Asset with this identifier not found");
                    }
                }
                parsed=true;
            }
            if(d.name_ == "key")
            {
                if(d.value_.type() != null_type && (d.value_.type()==str_type))
                {
                    vKey=vector<unsigned char>(d.value_.get_str().begin(), d.value_.get_str().end());
//                    key_is_set=true;
                }
                else
                {
                    strError=string("Invalid key");
                }
                if((allowed_objects & 0x0002) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }
            if(d.name_ == "key-hex")
            {
                if(d.value_.type() != null_type && (d.value_.type()==str_type))
                {
                    bool fIsHex;
                    vKey=ParseHex(d.value_.get_str().c_str(),fIsHex);
                    if(!fIsHex)
                    {
                        strError=string("key should be hexadecimal string");
                    }
//                    key_is_set=true;
                }
                else
                {
                    strError=string("Invalid key");
                }
                if((allowed_objects & 0x0002) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }
            if(d.name_ == "data")
            {
                if(d.value_.type() != null_type && (d.value_.type()==str_type))
                {
                    bool fIsHex;
                    vValue=ParseHex(d.value_.get_str().c_str(),fIsHex);
                    if(!fIsHex)
                    {
                        strError=string("value should be hexadecimal string");
                    }
//                    value_is_set=true;
                }
                else
                {
                    strError=string("Invalid value");
                }
                if((allowed_objects & 0x0002) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }

            if(d.name_ == "name")
            {
                if(d.value_.type() != null_type && !d.value_.get_str().empty())
                {
                    entity_name=d.value_.get_str().c_str();
                }
                else
                {
                    strError=string("Invalid name");
                }
                if((allowed_objects & 0x0005) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }
            if(d.name_ == "multiple")
            {
                if(d.value_.type() == int_type)
                {
                    multiple=d.value_.get_int();
                    multiple_is_set=true;
                }
                else
                {
                    strError=string("Invalid multiple");
                }
                if((allowed_objects & 0x0004) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }
            if(d.name_ == "startblock")
            {
                if(d.value_.type() == int_type)
                {
                    if( (d.value_.get_int64() >= 0) && (d.value_.get_int64() <= 0xFFFFFFFF) )
                    {
                        startblock=(uint32_t)(d.value_.get_int64());
                        startblock_is_set=true;
                    }
                    else
                    {
                        strError=string("Invalid startblock");
                    }
                }
                else
                {
                    strError=string("Invalid startblock");
                }
                if((allowed_objects & 0x0040) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }
            if(d.name_ == "open")
            {
                if(d.value_.get_bool())
                {
                    is_open=1;
                }
                if((allowed_objects & 0x0005) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                open_is_set=true;
                parsed=true;
            }
            if(d.name_ == "approve")
            {
                if(d.value_.get_bool())
                {
                    approve=1;
                }
                else
                {
                    approve=0;
                }
                if((allowed_objects & 0x0020) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }
            if(d.name_ == "details")
            {
                if(new_type == -3)
                {
                    strError=string("details and inputcache not allowed in the same object");
                }
                if(d.value_.type() == obj_type)
                {
                    //BOOST_FOREACH(const Pair& p, d.value_.get_obj())
                    for (const Pair& p : d.value_.get_obj())
                    {
                        if( (p.name_ == "protocol-version") && (p.value_.type() == int_type) )
                        {
                            if( p.value_.get_int() > 0 )
                            {
                                protocol_version=p.value_.get_int();
                            }
                            else
                            {
                                strError=string("Invalid protocol-version");
                            }
                        }
                        else
                        {
                            lpDetails->SetParamValue(p.name_.c_str(),p.name_.size(),(unsigned char*)p.value_.get_str().c_str(),p.value_.get_str().size());
                        }
                    }
                }
                if((allowed_objects & 0x000D) == 0)
                {
                    strError=string("Keyword not allowed in this API");
                }
                parsed=true;
            }
            if(!parsed)
            {
                strError=string("Invalid field: ") + d.name_;
            }
        }

        if(strError.size() == 0)
        {
            if(new_type == 0)
            {
//                strError=string("One of the following keywords can appear in the object: create, update, for");
                if(given_entity && given_entity->GetEntityType())
                {
                    memcpy(&entity,given_entity,sizeof(mc_EntityDetails));
                    new_type=-2;
                }
                else
                {
                    new_type=MC_ENT_TYPE_ASSET;
                }
            }
        }

        if(strError.size() == 0)
        {
            if(new_type == -2)
            {
                if((allowed_objects & 0x0008) == 0)
                {
                    strError=string("Follow-on issuance not allowed in this API");
                }
                else
                {
                    if(vKey.size())
                    {
                        strError=string("Invalid field: key");
                    }
                    if(vValue.size())
                    {
                        strError=string("Invalid field: value");
                    }
                }
            }
        }

        if(strError.size() == 0)
        {
            if(new_type == MC_ENT_TYPE_ASSET)
            {
                if((allowed_objects & 0x0004) == 0)
                {
                    strError=string("Issuing new assets not allowed in this API");
                }
                else
                {
                    if(is_open)
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_FOLLOW_ONS,(unsigned char*)&is_open,1);
                    }
                    if(multiple_is_set)
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_ASSET_MULTIPLE,(unsigned char*)&multiple,4);
                    }
                    if(entity_name.size())
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_NAME,(const unsigned char*)(entity_name.c_str()),entity_name.size());//+1);
                    }
                    if(vKey.size())
                    {
                        strError=string("Invalid field: key");
                    }
                    if(vValue.size())
                    {
                        strError=string("Invalid field: value");
                    }
                }
            }
        }

        if(strError.size() == 0)
        {
            if(new_type == MC_ENT_TYPE_STREAM)
            {
                if((allowed_objects & 0x0001) == 0)
                {
                    strError=string("Creating new streams not allowed in this API");
                }
                else
                {
                    format=1;
                    if(entity_name.size())
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_NAME,(const unsigned char*)(entity_name.c_str()),entity_name.size());//+1);
                    }
                    if(is_open)
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_ANYONE_CAN_WRITE,(unsigned char*)&is_open,1);
                    }
                    if(multiple_is_set)
                    {
                        strError=string("Invalid field: multiple");
                    }
                    if(vKey.size())
                    {
                        strError=string("Invalid field: key");
                    }
                    if(vValue.size())
                    {
                        strError=string("Invalid field: value");
                    }
                }
            }
            if(new_type == -1)
            {
                format=-1;
            }
        }

        if(strError.size() == 0)
        {
            if(new_type != MC_ENT_TYPE_UPGRADE)
            {
                if(protocol_version > 0)
                {
                    strError=string("Invalid field: protocol-version");
                }
                if(startblock_is_set)
                {
                    strError=string("Invalid field: startblock");
                }
            }
            if(new_type != -5)
            {
                if(approve >= 0)
                {
                    strError=string("Invalid field: approve");
                }
            }
        }

        if(strError.size() == 0)
        {
            if(new_type == MC_ENT_TYPE_UPGRADE)
            {
                if((allowed_objects & 0x0040) == 0)
                {
                    strError=string("Creating new upgrades not allowed in this API");
                }
                else
                {
                    if(lpDetails->m_Size)
                    {
                        strError=string("Invalid fields in details object");
                    }
                    if(entity_name.size())
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_NAME,(const unsigned char*)(entity_name.c_str()),entity_name.size());//+1);
                    }
                    if(protocol_version > 0)
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_UPGRADE_PROTOCOL_VERSION,(unsigned char*)&protocol_version,4);
                    }
                    else
                    {
                        strError=string("Missing protocol-version");
                    }
                    if(startblock > 0)
                    {
                        lpDetails->SetSpecialParamValue(MC_ENT_SPRM_UPGRADE_START_BLOCK,(unsigned char*)&startblock,4);
                    }
                    if(multiple_is_set)
                    {
                        strError=string("Invalid field: multiple");
                    }
                    if(open_is_set)
                    {
                        strError=string("Invalid field: open");
                    }
                    if(vKey.size())
                    {
                        strError=string("Invalid field: key");
                    }
                    if(vValue.size())
                    {
                        strError=string("Invalid field: value");
                    }
                }
            }
        }

        if(strError.size() == 0)
        {
            if(new_type == -5)
            {
                if(lpDetails->m_Size)
                {
                    strError=string("Invalid field: details");
                }
                if(multiple_is_set)
                {
                    strError=string("Invalid field: multiple");
                }
                if(open_is_set)
                {
                    strError=string("Invalid field: open");
                }
                if(vKey.size())
                {
                    strError=string("Invalid field: key");
                }
                if(vValue.size())
                {
                    strError=string("Invalid field: value");
                }
            }
        }

        if(strError.size() == 0)
        {
            int err;
            size_t bytes;
            const unsigned char *script;

            if(new_type == MC_ENT_TYPE_ASSET)
            {
                script=lpDetails->GetData(0,&bytes);
                err=lpDetailsScript->SetNewEntityType(MC_ENT_TYPE_ASSET,0,script,bytes);
                if(err)
                {
                    strError=string("Invalid custom fields, too long");
                }
                else
                {
                    script = lpDetailsScript->GetData(0,&bytes);
                    scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP << OP_RETURN;
                }
            }

            if(new_type == MC_ENT_TYPE_STREAM)
            {
                int err;
                script=lpDetails->GetData(0,&bytes);
                err=lpDetailsScript->SetNewEntityType(MC_ENT_TYPE_STREAM,0,script,bytes);
                if(err)
                {
                    strError=string("Invalid custom fields, too long");
                }
                else
                {
                    script = lpDetailsScript->GetData(0,&bytes);
                    scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP << OP_RETURN;
                }
            }

            if(new_type == MC_ENT_TYPE_UPGRADE)
            {
                int err;
                script=lpDetails->GetData(0,&bytes);
                err=lpDetailsScript->SetNewEntityType(MC_ENT_TYPE_UPGRADE,0,script,bytes);
                if(err)
                {
                    strError=string("Invalid custom fields, too long");
                }
                else
                {
                    script = lpDetailsScript->GetData(0,&bytes);
                    scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP << OP_RETURN;
                }
            }

            if(new_type == -2)
            {
                int err;
                lpDetailsScript->Clear();
                lpDetailsScript->SetEntity(entity.GetTxID()+MC_AST_SHORT_TXID_OFFSET);
                script = lpDetailsScript->GetData(0,&bytes);
                scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP;

                lpDetailsScript->Clear();
                script=lpDetails->GetData(0,&bytes);
                err=lpDetailsScript->SetNewEntityType(MC_ENT_TYPE_ASSET,1,script,bytes);
                if(err)
                {
                    strError=string("Invalid custom fields, too long");
                }
                else
                {
                    script = lpDetailsScript->GetData(0,&bytes);
                    scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP << OP_RETURN;
                }
            }

            if(new_type == -1)
            {
                lpDetailsScript->Clear();
                lpDetailsScript->SetEntity(entity.GetTxID()+MC_AST_SHORT_TXID_OFFSET);
                script = lpDetailsScript->GetData(0,&bytes);
                scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP;

                lpDetailsScript->Clear();
                if(lpDetailsScript->SetItemKey(&vKey[0],vKey.size()) == MC_ERR_NOERROR)
                {
                    script = lpDetailsScript->GetData(0,&bytes);
                    scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP;
                }

                scriptOpReturn << OP_RETURN << vValue;
            }

            if(new_type == -3)
            {
                script=lpDetails->GetData(0,&bytes);
                scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP << OP_RETURN;
            }

            if(new_type == -5)
            {
                if(approve < 0)
                {
                    strError=string("Missing approve field");
                }

                lpDetailsScript->Clear();
                lpDetailsScript->SetEntity(entity.GetTxID()+MC_AST_SHORT_TXID_OFFSET);
                script = lpDetailsScript->GetData(0,&bytes);
                scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP;

                lpDetailsScript->Clear();
                lpDetailsScript->SetApproval(approve, mc_TimeNowAsUInt());
                script = lpDetailsScript->GetData(0,&bytes);
                scriptOpReturn << vector<unsigned char>(script, script + bytes) << OP_DROP;

                scriptOpReturn << OP_RETURN;
            }
        }

        delete lpDetails;
        delete lpDetailsScript;
        if(strError.size())
        {
            throw to_string(RPC_INVALID_PARAMETER) + ": " + strError;
        }
    }
    else
    {
        bool fIsHex;
        if( ((allowed_objects & 0x0100) != 0) || (param.get_str().size() != 0) )
        {
            vector<unsigned char> dataData(ParseHex(param.get_str().c_str(),fIsHex));
            if(!fIsHex)
            {
                throw to_string(RPC_INVALID_PARAMETER) + ": " + "data-hex should be hexadecimal string or recognized object format";
            }
            scriptOpReturn << OP_RETURN << dataData;
        }
    }

    return scriptOpReturn;
}

