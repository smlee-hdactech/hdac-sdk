/**     
* @file     hs_keys.cpp
* @date     2019-1-17
* @author   HDAC Technology Inc.
*
* @brief    hs_keys 소스파일.
*/

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

/**
 *
 * @brief 개인키를 생성한다.
 * @details 개인키와 함께 공개키 및 이에 따른 공개키 해시와 지갑주소도 함께 구한다.
 * @param privateHelper 개인키 처리를 위한 정보 제공 인터페이스
 * @param addrHelper 지갑주소 처리를 위한 정보 제공 인터페이스
 *
 * @return KeyPairs
 *
 */
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

/**
 *
 * @brief 개인키 처리를 위한 정보 제공 인터페이스를 가져온다.
 * @details 주로 개인키 처리를 위해 내부적으로 사용된다. 
 * @details UTXO에 대한 정보는 listunspent RPC 명령을 통해 가져온다.
 * @details 발행한 자산에 대한 정보는 listassets RPC 명령을 통해 가져온다.
 * @details 개인키는 createKeyPairs 함수나 RPC 명령을 통해 가져온다.
 * @param toAddr	보낼 지갑 주소
 * @param quantity	보낼 자산량
 * @param issueTxid		발행한 트랜잭션 ID
 * @param multiple		자산에 대한 multiple 값
 * @param unspentScriptPubKey	UTXO에 대한 scriptPubKey
 * @param unspentTxid			UTXO에 대한 트랜잭션 ID
 * @param unspentVOut			UTXO의 인덱스
 * @param unspentQty			UTXO의 양
 * @param unspentRedeemScript	UTXO의 redeem script (muti-sig 용으로 주로 사용)
 * @param privateKey			보내는 지갑에 대한 개인키
 * @param privateHelper			개인키 처리를 위한 정보 제공 인터페이스
 * @param walletHelper			지갑주소 처리를 위한 정보 제공 인터페이스
 *
 * @return 개인키 처리를 위한 정보 제공 인터페이스
 *
 */
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

/**
 *
 * @brief 스트림키 발행을 위한 raw-tx 문자열을 생성한다.
 * @details 보안을 강화하기 위해, 블록체인 망 내의 노드를 이용하지 않고, 표출되지 않은 개인키와 망으로 부터 구한 블록체인의 트랜잭션 정보들로부터 직접 생성한다.
 * @details  주로 개인키 처리를 위해 내부적으로 사용된다.
 * @details  UTXO에 대한 정보는 listunspent RPC 명령을 통해 가져온다.
 * @details  생성한 스트림에 대한 정보는 liststreams RPC 명령을 통해 가져온다.
 * @details  개인키는 createKeyPairs 함수나 RPC 명령을 통해 가져온다.
 * @param streamKey	스트림에 대한 키
 * @param streamItem	키에 대한 값
 * @param createTxid		스트림 생성 트랜잭션 ID
 * @param unspentScriptPubKey	UTXO에 대한 scriptPubKey
 * @param unspentTxid			UTXO에 대한 트랜잭션 ID
 * @param unspentVOut			UTXO의 인덱스
 * @param unspentRedeemScript	UTXO의 redeem script (muti-sig 용으로 주로 사용)
 * @param privateKey			보내는 지갑에 대한 개인키
 * @param privateHelper			개인키 처리를 위한 정보 제공 인터페이스
 *
 * @return raw-tx 문자열
 *
 */
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

/**
 *
 * @brief 공개키로부터 지갑주소를 문자열로 얻는다.
 * @details 문자열로 표기된 공개키와 주소를 생성하기 위한 prefix값과 checksum을 통해서 지갑주소를 구해낸다.
 * @param pubkeyStr 문자열로 표기된 공개키
 * @param addrHelper 지갑주소 처리를 위한 정보 제공 인터페이스
 * @return 문자열로 표기된 지갑주소
 *
 */
string walletAddrFromPubKey(const string& pubkeyStr, const IWalletAddrHelper& addrHelpler)
{
    CPubKey pubkey(ParseHex(pubkeyStr));
    CBitcoinAddress addr(pubkey.GetID(), addrHelpler);
    return addr.ToString();
}

/**
 *
 * @brief 개인키 또는 지갑주소로 sign 된 메시지를 검증 한다.
 * @details 개인키 또는 지갑주소로 sign 된 메시지가 해당 개인키 또는 지갑주소로 제대로 sign 되었는지 검증 할때 사용 한다.
 * @param strAddress sign 할 때 사용 된 지갑주소
 * @param strSign sign 되어진 문자열
 * @param strMessage 원본 문자열
 * @param addrHelper 지갑주소 처리를 위한 정보 제공 인터페이스
 * 
 * @return 해당 개인키 또는 지갑주소로 sign 메시지가 맞다면 true 아니라면 false
 *
 */
bool VerifyMessage(const string &strAddress, const string &strSign, const string &strMessage, const IWalletAddrHelper &addrHelper)
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

/**
 *
 * @brief 개인키를 이용하여 sign 된 메시지를 만든다.
 * @details 개인키를 이용하여 평문으로 된 메시지를 sign 하여 암호화 한다.
 * @param strAddress sign 할려고 하는 개인키 값
 * @param strMessage sign 할려고 하는 원본 문자열
 * @param privateHelper 개인키 처리를 위한 정보 제공 인터페이스
 * @param addrHelper 지갑주소 처리를 위한 정보 제공 인터페이스
 * 
 * @return base64 로 인코딩 된 sign 된 문자열
 *
 */
string SignMessage(const string &strAddress, const string &strMessage,
		const IPrivateKeyHelper &privateHelper, const IWalletAddrHelper &addrHelper)
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

		return "it is insert address type, but not yet make";
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

