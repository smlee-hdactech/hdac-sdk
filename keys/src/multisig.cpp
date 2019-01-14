#include "multisig.h"
#include <json_spirit/json_spirit.h>
#include "pubkey.h"
#include <utils/utilstrencodings.h>
#include <utils/tinyformat.h>
#include "keyshelper.h"
#include "bitcoinaddress.h"

using namespace std;
using namespace json_spirit;
// TODO : namespace for hdac-sdk

namespace
{
class CScriptVisitor : public boost::static_visitor<bool>
{
private:
    CScript *script;
public:
    CScriptVisitor(CScript *scriptin) { script = scriptin; }

    bool operator()(const CNoDestination &dest) const {
        script->clear();
        return false;
    }

    bool operator()(const CKeyID &keyID) const {
        script->clear();
        *script << OP_DUP << OP_HASH160 << ToByteVector(keyID) << OP_EQUALVERIFY << OP_CHECKSIG;
        return true;
    }

    bool operator()(const CScriptID &scriptID) const {
        script->clear();
        *script << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
        return true;
    }
};
}

CScript GetScriptForDestination(const CTxDestination& dest)
{
    CScript script;

    boost::apply_visitor(CScriptVisitor(&script), dest);
    return script;
}

// from script/standard.cpp, owing to hierarchy
CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys)
{
    CScript script;

    script << CScript::EncodeOP_N(nRequired);
    for (const CPubKey& key : keys) {
        script << ToByteVector(key);
    }
    script << CScript::EncodeOP_N(keys.size()) << OP_CHECKMULTISIG;
    return script;
}

/**
 * Used by addmultisigaddress / createmultisig:
 */
// TODO : should get onlyAcceptStdTx, maxStdElemSize from blockchain-params
CScript createMultisigRedeemScript(int nRequired, const vector<string>& keys,
                                   bool onlyAcceptStdTx, unsigned int maxStdElemSize)
{
    // Gather public keys
    if (nRequired < 1)
        throw runtime_error("a multisignature address must require at least one key to redeem");
    if ((int)keys.size() < nRequired)
        throw runtime_error(
            strprintf("not enough keys supplied "
                      "(got %u keys, but need at least %d to redeem)", keys.size(), nRequired));
    if (keys.size() > 16)
        throw runtime_error("Number of addresses involved in the multisignature address creation > 16\nReduce the number");
    std::vector<CPubKey> pubkeys;
    pubkeys.resize(keys.size());
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        const std::string& ks = keys[i];
#if 0   // TODO : have key DB?
        // Case 1: Bitcoin address and we have full public key:
        CBitcoinAddress address(ks);
        if (pwalletMain && address.IsValid())
        {
            CKeyID keyID;
            if (!address.GetKeyID(keyID))
                throw runtime_error(
                    strprintf("%s does not refer to a key",ks));
            CPubKey vchPubKey;
            if (!pwalletMain->GetPubKey(keyID, vchPubKey))
                throw runtime_error(
                    strprintf("no full public key for address %s",ks));
            if (!vchPubKey.IsFullyValid())
                throw runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }

        // Case 2: hex public key
        else
#endif
        if (IsHex(ks))
        {
            CPubKey vchPubKey(ParseHex(ks));
            if (!vchPubKey.IsFullyValid())
                throw runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }
        else
        {
            throw runtime_error(" Invalid public key: "+ks);
        }
    }
    CScript result = GetScriptForMultisig(nRequired, pubkeys);

    if(onlyAcceptStdTx)
    {
        if (result.size() > maxStdElemSize)
            throw runtime_error(
                    strprintf("redeemScript exceeds size limit: %d > %d", result.size(), maxStdElemSize));
    }

    return result;
}

void createMultisigInfo(const vector<string>& pubkeys, int required, const IWalletAddrHelper& addrHelper, MultisigAddrInfo& info)
{
    // "027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b","038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd"

    CScript script = createMultisigRedeemScript(required, pubkeys);
    //cout << "MultisigRedeemScript: ";
    //cout << HexStr(script) << endl;

    CScriptID scriptID(script);
    //cout << "MultisigRedeemScript ID: ";
    //cout << HexStr(scriptID) << endl;

    CScript outer = GetScriptForDestination(scriptID);
    //cout << "outer: " << HexStr(outer) << endl;

    //return CBitcoinAddress(innerID).ToString();
    //cout << "multisig addr: ";
    //cout << CBitcoinAddress(scriptID, SampleWalletAddrHelper()).ToString() << endl;
    info.addr = CBitcoinAddress(scriptID, addrHelper).ToString();
    info.redeemScript = HexStr(script);
}
