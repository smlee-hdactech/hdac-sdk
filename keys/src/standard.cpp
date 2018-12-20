#include "standard.h"
#include <primitives/transaction.h>
#include "pubkey.h"
#include <script/standard.h>
#include <boost/variant.hpp>

using namespace std;

typedef vector<unsigned char> valtype;

// TODO : MAX_OP_RETURN_RELAY is from maxstdopreturnsize, -datacarriersize
static unsigned int MAX_OP_RETURN_RELAY = 40;

// TODO : MAX_SCRIPT_ELEMENT_SIZE is from maxstdelementsize
static unsigned int MAX_SCRIPT_ELEMENT_SIZE=520;                                       // script.h

CTxDestination VectorToAddress(vector<unsigned char>& vch)
{
    CTxDestination addressRet;
    addressRet=CNoDestination();

    if( (vch.size() >= 33) && (vch.size() <= 65) )
    {
        CPubKey pubKey(vch.begin(), vch.end());
        if (pubKey.IsValid())
        {
            addressRet = pubKey.GetID();
        }
    }

    return addressRet;
}

bool IsStandardNullData(const CScript& scriptPubKey,bool standard_check)
{
    opcodetype opcode;
    vector<unsigned char> vch;
    bool recheck=false;
    bool fixed=true;
    int op_drop_count=0;
    int max_op_drop_count=2;
    unsigned int sizes[2];
    sizes[0]=0;
    sizes[1]=0;

    CScript::const_iterator pc = scriptPubKey.begin();

    while( op_drop_count < max_op_drop_count+1 )
    {
        if(!scriptPubKey.GetOp(pc, opcode))
        {
            return false;
        }

        if(opcode == OP_RETURN)
        {
            op_drop_count=max_op_drop_count+1;
        }
        else
        {
            if(op_drop_count == max_op_drop_count)
            {
                return false;
            }
        }

        if(op_drop_count < max_op_drop_count+1)
        {
            if( opcode < OP_PUSHDATA1 )
            {
                sizes[op_drop_count]=(unsigned int)opcode;
            }
            else
            {
                if( opcode <= OP_PUSHDATA4 )
                {
                    if( !fixed || standard_check )
                    {
                        recheck=true;
                    }
                }
                else
                {
                    if(fixed)
                    {
                        return false;
                    }
                }
            }

            if(!scriptPubKey.GetOp(pc, opcode))
            {
                return false;
            }
            if(opcode != OP_DROP)
            {
                return false;
            }
            op_drop_count++;
        }
    }

    if (pc < scriptPubKey.end())
    {
        if(pc + OP_PUSHDATA1 < scriptPubKey.end())
        {
            scriptPubKey.GetOp(pc, opcode, vch);
            if( !fixed || standard_check )
            {
                if (vch.size() > MAX_OP_RETURN_RELAY)
                {
                    return false;
                }
            }
        }
        else
        {
            scriptPubKey.GetOp(pc, opcode);
            if(opcode >= OP_PUSHDATA1)
            {
                return false;
            }
            if( !fixed || standard_check )
            {
                if ((unsigned int)opcode > MAX_OP_RETURN_RELAY)
                {
                    return false;
                }
            }
        }
        if(scriptPubKey.GetOp(pc, opcode))
        {
            return false;
        }
    }

    if( !fixed || standard_check )
    {
        if(recheck)
        {
            pc = scriptPubKey.begin();

            op_drop_count=0;
            while( op_drop_count < max_op_drop_count+1 )
            {
                scriptPubKey.GetOp(pc, opcode, vch);
                if(opcode == OP_RETURN)
                {
                    op_drop_count=max_op_drop_count+1;
                }
                if( opcode >= OP_PUSHDATA1 )
                {
                    if( opcode <= OP_PUSHDATA4 )
                    {
                        sizes[op_drop_count]=(unsigned int)vch.size();
                    }
                }
                if(op_drop_count < max_op_drop_count+1)
                {
                    scriptPubKey.GetOp(pc, opcode);
                    op_drop_count++;
                }
            }
        }

        for(op_drop_count=0;op_drop_count<max_op_drop_count;op_drop_count++)
        {
            if( sizes[op_drop_count] > MAX_SCRIPT_ELEMENT_SIZE )
            {
                return false;
            }
        }
    }

    return true;
}

bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, vector<CTxDestination>& addressRet, int& nRequiredRet,vector<vector<unsigned char> >* lpvSolutionsRet)
{
    addressRet.clear();
    typeRet = TX_NONSTANDARD;
    opcodetype opcode;
    vector<unsigned char> vch;
    CTxDestination pkAddress;
    int n;

    nRequiredRet=1;

    CScript::const_iterator pc = scriptPubKey.begin();

    if (scriptPubKey.GetOp(pc, opcode))
    {
        if(opcode == OP_DUP)                                                    // pay-to-pubkeyhash
        {
            if ( !scriptPubKey.GetOp(pc, opcode) || (opcode != OP_HASH160) )
            {
                return false;
            }
            if ( !scriptPubKey.GetOp(pc, opcode, vch) || (vch.size() != 20) )
            {
                return false;
            }
            addressRet.push_back(CKeyID(uint160(vch)));
            if(lpvSolutionsRet)
            {
                lpvSolutionsRet->push_back(vch);
            }
            if ( !scriptPubKey.GetOp(pc, opcode) || (opcode != OP_EQUALVERIFY) )
            {
                return false;
            }
            if ( !scriptPubKey.GetOp(pc, opcode) || (opcode != OP_CHECKSIG) )
            {
                return false;
            }
            typeRet = TX_PUBKEYHASH;
        }
        else
        {
            if(opcode == OP_HASH160)                                            // pay-to-scripthash
            {
                if ( !scriptPubKey.GetOp(pc, opcode, vch) || (vch.size() != 20) )
                {
                    return false;
                }
                addressRet.push_back(CScriptID(uint160(vch)));
                if(lpvSolutionsRet)
                {
                    lpvSolutionsRet->push_back(vch);
                }
                if ( !scriptPubKey.GetOp(pc, opcode) || (opcode != OP_EQUAL) )
                {
                    return false;
                }
                typeRet = TX_SCRIPTHASH;
            }
            else
            {
                if(IsStandardNullData(scriptPubKey,false))                      // null-data
                {
                    typeRet=TX_NULL_DATA;
                    return false;
                }
                else
                {
                    pc = scriptPubKey.begin();
                    if ( !scriptPubKey.GetOp(pc, opcode, vch) )
                    {
                        return false;
                    }

                    pkAddress=VectorToAddress(vch);
                    if( boost::get<CKeyID> (&pkAddress) )                       // pay-to-pubkey
                    {
                        if ( !scriptPubKey.GetOp(pc, opcode) || (opcode != OP_CHECKSIG) )
                        {
                            return false;
                        }
                        addressRet.push_back(pkAddress);
                        if(lpvSolutionsRet)
                        {
                            lpvSolutionsRet->push_back(vch);
                        }
                        typeRet = TX_PUBKEY;
                    }
                    else
                    {
                        if ( (opcode >= OP_1 && opcode <= OP_16) )              // bare multisig
                        {
                            nRequiredRet=CScript::DecodeOP_N(opcode);
                            if(lpvSolutionsRet)
                            {
                                lpvSolutionsRet->push_back(valtype(1, (char)nRequiredRet));
                            }

                            n=-1;
                            while(n != (int)addressRet.size())
                            {
                                if ( !scriptPubKey.GetOp(pc, opcode, vch) )
                                {
                                    return false;
                                }
                                if ( (opcode >= OP_1 && opcode <= OP_16) )
                                {
                                    n=CScript::DecodeOP_N(opcode);
                                    if(n != (int)addressRet.size())
                                    {
                                        return false;
                                    }
                                }
                                else
                                {
                                    pkAddress=VectorToAddress(vch);
                                    if( boost::get<CKeyID> (&pkAddress) )
                                    {
                                        addressRet.push_back(pkAddress);
                                        if(lpvSolutionsRet)
                                        {
                                            lpvSolutionsRet->push_back(vch);
                                        }
                                    }
                                    else
                                    {
                                        return false;
                                    }
                                }
                            }
                            if(lpvSolutionsRet)
                            {
                                lpvSolutionsRet->push_back(valtype(1, (char)n));
                            }


                            if ( !scriptPubKey.GetOp(pc, opcode) || (opcode != OP_CHECKMULTISIG) )
                            {
                                return false;
                            }
                            typeRet = TX_MULTISIG;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }

    while(pc < scriptPubKey.end())
    {
        if ( !scriptPubKey.GetOp(pc, opcode) || (opcode > OP_PUSHDATA4) )
        {
            typeRet = TX_NONSTANDARD;
            return false;
        }
        if ( !scriptPubKey.GetOp(pc, opcode) || (opcode != OP_DROP) )
        {
            typeRet = TX_NONSTANDARD;
            return false;
        }
    }

    return true;
}

bool TemplateSolver(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<std::vector<unsigned char> >& vSolutionsRet)
{
    vector<CTxDestination> addressRet;
    int nRequiredRet;

    return ExtractDestinations(scriptPubKey,typeRet,addressRet,nRequiredRet,&vSolutionsRet);
}
