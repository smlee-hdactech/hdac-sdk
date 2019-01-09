#include "hs_keys.h"
#include <utils/utilstrencodings.h>
#include <utils/base64.h>
#include <algorithm>
#include <cstring>
#include <entities/asset.h>
#include <memory>
#include "bitcoinaddress.h"
#include <script/standard.h>
#include <script/hdacscript.h>
#include "multisig.h"
#include "key.h"
#include <rpc/rpcprotocol.h>
#include <primitives/transaction.h>
#include "transactions.h"
#include <primitives/interpreter.h>
#include <standard.h>
#include <utils/define.h>
#include "eccautoinitreleasehandler.h"
#include "bitcoinsecret.h"

using namespace std;

const string strMessageMagic = "Hdac Signed Message:\n"; // for verify message

KeyPairs createKeyPairs(const IPrivateKeyHelper &privateHelper, const IWalletAddrHelper &addrHelper)
{
    CKey secret;
    //secret.MakeNewKey(fCompressed);
    secret.MakeNewKey(true);

    EccAutoInitReleaseHandler eccScoper;
    CPubKey pubkey = secret.GetPubKey();

    CBitcoinAddress addr(pubkey.GetID(), addrHelper);
    string privateKeyStr = CBitcoinSecret(secret, privateHelper).ToString();
    string pubkeyStr = HexStr(pubkey);
    string pubkeyHashStr = HexStr(pubkey.GetID());
    string walletAddrStr = addr.ToString();

    return KeyPairs{privateKeyStr, pubkeyStr, pubkeyHashStr, walletAddrStr};
}

// TODO : implement
// create
// publish : single -> O, multisig -> X
// issue
// send asset

#define MC_AST_ASSET_REF_TYPE_OFFSET        32
#define MC_AST_ASSET_REF_TYPE_SIZE           4

string createAssetSendTx(const string& toAddr, double quantity,
                         const string& issueTxid, int multiple,
                         const string& unspentScriptPubKey, const string& unspentTxid, uint32_t unspentVOut,
                         double unspentQty,
                         const string& unspentRedeemScript, const string& privateKey,
                         const IPrivateKeyHelper& privateHelper, const IWalletAddrHelper& walletHelper)
{
    auto hexTxid = ParseHex(issueTxid);
    std::reverse(hexTxid.begin(), hexTxid.end());

    unsigned char buf[MC_AST_ASSET_FULLREF_BUF_SIZE];
    memset(buf, 0, MC_AST_ASSET_FULLREF_BUF_SIZE);
    memcpy(buf+MC_AST_SHORT_TXID_OFFSET, hexTxid.data()+MC_AST_SHORT_TXID_OFFSET, MC_AST_SHORT_TXID_SIZE);

    uint32_t type = MC_AST_ASSET_REF_TYPE_SHORT_TXID;
    mc_PutLE((unsigned char*)buf+MC_AST_ASSET_REF_TYPE_OFFSET,&type,MC_AST_ASSET_REF_TYPE_SIZE);

    int64_t qnt = (int64_t)(quantity * multiple + 0.499999);
    mc_PutLE((unsigned char*)buf+MC_AST_ASSET_QUANTITY_OFFSET,&qnt,MC_AST_ASSET_QUANTITY_SIZE);

    std::unique_ptr<mc_Buffer> buffer(new mc_Buffer);
    mc_InitABufferDefault(buffer.get());
    buffer->Clear();
    buffer->Add(buf);

    std::unique_ptr<mc_Script> dropScript(new mc_Script);
    dropScript->Clear();

    dropScript->SetAssetQuantities(buffer.get(), MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER);

    CBitcoinAddress toWalletAddress(toAddr, walletHelper);
    CTxDestination addressRet = toWalletAddress.Get();

    CScript scriptSend=GetScriptForDestination(addressRet);
    //CKeyID keyIdAddrRet = boost::get<CKeyID>(addressRet);
    //cout << "toAddr key: " << keyIdAddrRet.ToString() << endl;

    //CScript scriptPubKey=GetScriptForDestination(addressRet);

    size_t elem_size;
    const unsigned char *elem;

    for(int element=0;element < dropScript->GetNumElements();element++)
    {
        elem = dropScript->GetData(element,&elem_size);
        if(elem)    {
            //scriptPubKey << vector<unsigned char>(elem, elem + elem_size) << OP_DROP;
            scriptSend << vector<unsigned char>(elem, elem + elem_size) << OP_DROP;
        }
        else    {
            throw to_string(RPC_INTERNAL_ERROR) + ": Invalid script";
        }
    }


#if 0
    std::unique_ptr<mc_Buffer> change_amounts(new mc_Buffer);
    change_amounts->Initialize(MC_AST_ASSET_QUANTITY_OFFSET,MC_AST_ASSET_FULLREF_BUF_SIZE+sizeof(int),MC_BUF_MODE_MAP);
    change_amounts->Clear();
#endif

    // TODO : need to handle multiple scriptPubKeys
    CMutableTransaction txNew;

    CTxOut txout2(0, scriptSend);
    txNew.vout.push_back(txout2);

    CKey keyFromPriv = keyFromPrivateKey(privateKey, privateHelper);
    CScript scriptChange;
    {
        EccAutoInitReleaseHandler eccScoper;
        CTxDestination changeAddr = keyFromPriv.GetPubKey().GetID();
        scriptChange=GetScriptForDestination(changeAddr);
    }

    int64_t changeQnt = (int64_t)(unspentQty * multiple - qnt);
    mc_PutLE((unsigned char*)buf+MC_AST_ASSET_QUANTITY_OFFSET,&changeQnt,MC_AST_ASSET_QUANTITY_SIZE);

    buffer->Clear();
    buffer->Add(buf);

    dropScript->Clear();
    dropScript->SetAssetQuantities(buffer.get(), MC_SCR_ASSET_SCRIPT_TYPE_TRANSFER);

    for(int element=0;element < dropScript->GetNumElements();element++)
    {
        elem = dropScript->GetData(element,&elem_size);
        if(elem)    {
            //scriptPubKey << vector<unsigned char>(elem, elem + elem_size) << OP_DROP;
            scriptChange << vector<unsigned char>(elem, elem + elem_size) << OP_DROP;
        }
        else    {
            // TODO : this is not related to rpc
            throw to_string(RPC_INTERNAL_ERROR) + ": Invalid script";
        }
    }

    CTxOut txout3(0, scriptChange);
    txNew.vout.push_back(txout3);

    auto pubkeyScript = ParseHex(unspentScriptPubKey);
    CTxOut txOut1(0, CScript(pubkeyScript.begin(), pubkeyScript.end()));
    const CScript& script1 = txOut1.scriptPubKey;

    txNew.vin.push_back(CTxIn(uint256(unspentTxid), unspentVOut));

    int nIn = 0;
    int nHashType = SIGHASH_ALL;

    uint256 hash = SignatureHash(script1, txNew, nIn, nHashType);
    cout << "hash: " << HexStr(hash) << endl;

    CTxIn& txin = txNew.vin[nIn];

    //vector<valtype> vSolutions;
    txnouttype whichType;
    CScript& scriptSigRet = txin.scriptSig;

    if (!solver(privateKey, privateHelper, script1, hash, nHashType, unspentRedeemScript, scriptSigRet, whichType)) {
        return "";
    };

    if (whichType == TX_SCRIPTHASH) {
        CScript subscript = txin.scriptSig;
        uint256 hash2 = SignatureHash(subscript, txNew, nIn, nHashType);
        //cout << "hash2: " << HexStr(hash2) << endl;
        {
            CScript& scriptSigRet = txin.scriptSig;
            bool solved = solver(privateKey, privateHelper, subscript, hash2, nHashType, unspentRedeemScript, scriptSigRet, whichType);
        }
        txin.scriptSig << static_cast<valtype>(subscript);
    }

    string hex=EncodeHexTx(txNew);
    //cout << "after sign, TxHex: " << hex << endl;

    return hex;
}

//#define CHECK_TOADDR
#ifdef CHECK_TOADDR
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
#endif

string createStreamPublishTx(const string& streamKey, const string& streamItem,
                     const string& createTxid,
                     const string& unspentScriptPubKey, const string& unspentTxid, uint32_t unspentVOut,
                     const string& unspentRedeemScript, const string& privateKey, const IPrivateKeyHelper& helper)
{

    //mc_EntityDetails found_entity;

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

    CMutableTransaction txNew;
    for (const auto& s : vecSend)   {
        CTxOut txout(s.second, s.first);
        txNew.vout.push_back(txout);
    }

    auto pubkeyScript = ParseHex(unspentScriptPubKey);

    CTxOut txOut1(0, CScript(pubkeyScript.begin(), pubkeyScript.end()));
    const CScript& script1 = txOut1.scriptPubKey;

    // TODO : check, where from destination
    CTxDestination addressRet;
    ExtractDestinationScriptValid(script1, addressRet);
#ifdef CHECK_TOADDR
    CBitcoinAddress addr(addressRet, SampleWalletAddrHelper());
    cout << "from addrRet: " << addr.ToString() << endl;
#endif

    CScript scriptChange=GetScriptForDestination(addressRet);
    CTxOut txout2(0, scriptChange);
    txNew.vout.push_back(txout2);

    txNew.vin.push_back(CTxIn(uint256(unspentTxid), unspentVOut));

    int nIn = 0;
    int nHashType = SIGHASH_ALL;

    uint256 hash = SignatureHash(script1, txNew, nIn, nHashType);
    //cout << "hash: " << HexStr(hash) << endl;

    CTxIn& txin = txNew.vin[nIn];

    //vector<valtype> vSolutions;
    txnouttype whichType;
    CScript& scriptSigRet = txin.scriptSig;

    if (!solver(privateKey, helper, script1, hash, nHashType, unspentRedeemScript, scriptSigRet, whichType)) {
        return "";
    };

    if (whichType == TX_SCRIPTHASH) {
        CScript subscript = txin.scriptSig;
        uint256 hash2 = SignatureHash(subscript, txNew, nIn, nHashType);
        //cout << "hash2: " << HexStr(hash2) << endl;
        {
            CScript& scriptSigRet = txin.scriptSig;
            bool solved = solver(privateKey, helper, subscript, hash2, nHashType, unspentRedeemScript, scriptSigRet, whichType);
        }
        txin.scriptSig << static_cast<valtype>(subscript);
    }

    string hex=EncodeHexTx(txNew);
    //cout << "after sign, TxHex: " << hex << endl;

    return hex;
}

string walletAddrFromPubKey(const string& pubkeyStr, const IWalletAddrHelper& addrHelpler)
{
    CPubKey pubkey(ParseHex(pubkeyStr));
    CBitcoinAddress addr(pubkey.GetID(), addrHelpler);
    return addr.ToString();
}

bool verifymessage(string strAddress, string strSign, string strMessage, const IWalletAddrHelper &addrHelper)
{
	EccAutoInitReleaseHandler eccScoper;

        CBitcoinAddress addr(strAddress, addrHelper);
        if (!addr.IsValid()) {
                cout << "addr error" << endl;
                return false;
        }

        CKeyID keyID;
        if (!addr.GetKeyID(keyID)) {
                cout << "get key id error" << endl;
                return false;
        }

        bool fInvalid = false;
        vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

        if (fInvalid) {
                cout << "decode base 64 error" << endl;
                return false;
        }

        CHashWriter ss(SER_GETHASH, 0);
        ss << strMessageMagic;
        ss << strMessage;

        CPubKey pubkey;
        if (!pubkey.RecoverCompact(ss.GetHash(), vchSig)) {
                return false;
        }

        return (pubkey.GetID() == keyID);
}


string signmessage(string strAddress, string strMessage, const IPrivateKeyHelper &privateHelper, const IWalletAddrHelper &addrHelper)
{
	EccAutoInitReleaseHandler eccScoper;

        CKey key;
        CBitcoinAddress addr(strAddress, addrHelper);
        if (!addr.IsValid()) {
                CBitcoinSecret vchSecret(privateHelper);
                bool fGood = vchSecret.SetString(strAddress);

                if (fGood)
                {
                        key = vchSecret.GetKey();
                        if (!key.IsValid()) {
                                fGood=false;
                        } else {
                                CPubKey pubkey = key.GetPubKey();
                                assert(key.VerifyPubKey(pubkey));

                        }
                }

                if(!fGood) {
			cout << "invalid address or private key, vchsecret error" << endl;
                }
        } else {

		return "it is insert address type, but not yet make...";
		// to make address type, pwalletMain value...
/*
                CKeyID keyID;
                if (!addr.GetKeyID(keyID)) {
			cout << "address dose not refer to key, get key failed" << endl;
                }


                if (!pwalletMain->GetKey(keyID, key)) {
                //      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key not available");
                }

*/
        }

        CHashWriter ss(SER_GETHASH, 0);
        ss << strMessageMagic;
        ss << strMessage;

        vector<unsigned char> vchSig;
        if (!key.SignCompact(ss.GetHash(), vchSig)) {
		cout << "sign compact error" << endl;
        }

    return EncodeBase64(&vchSig[0], vchSig.size());
}

