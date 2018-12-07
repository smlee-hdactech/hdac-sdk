#include <iostream>
#include <rpc/rpccaller.h>
#include <structs/hashes.h>
#include <rpc/cif_rpccall.h>
#include <keys/key.h>
#include <utils/utilstrencodings.h>
#include <keys/bitcoinaddress.h>
#include <utils/utilsfront.h>
#include <rpc/rpcresult.h>
#include <rpc/rpcclient.h>
#include <keys/bitcoinsecret.h>
#include <keys/keyshelper.h>

using namespace std;
using namespace json_spirit;

void testCalcSHA256()
{
    //using namespace std::placeholders;
    //obtainHash("This is test message", displayHashValue);
    // data from https://passwordsgenerator.net/sha256-hash-generator/
    auto compareHash = bind(compareHashValue, placeholders::_1, "DDDBDC2845C9D80DC288710D9B2CF2D6C4F613D0DC4C048A9EA0E8674C2C5E73");
    obtainHash("This is test message", compareHash);
//    auto compareHash1 = bind(compareHashValue, _1, "CDDBDC2845C9D80DC288710D9B2CF2D6C4F613D0DC4C048A9EA0E8674C2C5E73");
//    obtainHash("This is test message", compareHash1);
}

// TODO : should add the logging function
// TODO : parameters from config-file.
Object blockChainParams()
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    return client.CallRPC("getblockchainparams");
}

class KeysHelper {
public:
    KeysHelper() {
        const Object reply = blockChainParams();

        string resultStr;
        int nRet = result(reply, resultStr);
        if (nRet) {
            cerr << "rpc error : " << resultStr;
            return;
        }

        std::vector<string> keys{
            "address-pubkeyhash-version",
            "address-scripthash-version",
            "address-checksum-value",
            "private-key-version"
        };

        _resultMap = mapFromRpcResult(resultStr, keys);
        _addrHelper.reset(new WalletAddrHelper(_resultMap));
        _privHelper.reset(new PrivateKeyHelper(_resultMap));
    }

    IWalletAddrHelper& addrHelper() {
        return *_addrHelper;
    }

    IPrivateKeyHelper& privHelper() {
        return *_privHelper;
    }

private:
    map<string, string> _resultMap;

    class WalletAddrHelper : public IWalletAddrHelper {
    public:
        WalletAddrHelper(const map<string, string> &result) :
            _resultMap(result)  { }

        const std::vector<unsigned char> pubkeyAddrPrefix() const override  {
            return ParseHex(_resultMap.at("address-pubkeyhash-version"));
        }
        const std::vector<unsigned char> scriptAddrPrefix() const override  {
            return ParseHex(_resultMap.at("address-scripthash-version"));
        }

        int32_t addrChecksumValue() const override {
            return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
        }
    private:
        const map<string, string> & _resultMap;

    };
    std::unique_ptr<WalletAddrHelper> _addrHelper;

    class PrivateKeyHelper : public IPrivateKeyHelper {
    public:
        PrivateKeyHelper(const map<string, string> &result) :
            _resultMap(result)  { }

        const std::vector<unsigned char> privkeyPrefix() const override {
            return ParseHex(_resultMap.at("private-key-version"));
        }

        int32_t addrChecksumValue() const override  {
            return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
        }
    private:
        const map<string, string> &_resultMap;
    };
    std::unique_ptr<PrivateKeyHelper> _privHelper;
};


void createKeyPairs()
{
    using namespace std;

    ECC_Start();
    unique_ptr<ECCVerifyHandle> handle(new ECCVerifyHandle);

    if(!ECC_InitSanityCheck()) {
        cerr << "Elliptic curve cryptography sanity check failure. Aborting." << endl;
        //InitError("Elliptic curve cryptography sanity check failure. Aborting.");
        ECC_Stop();
        return;
    }

    CKey secret;
    //secret.MakeNewKey(fCompressed);
    secret.MakeNewKey(true);

    CPubKey pubkey = secret.GetPubKey();

    KeysHelper helper;

    CBitcoinAddress addr(pubkey.GetID(), helper.addrHelper());

    //signmessage with private key, and compare pubkeys each other

    cout << "address : " << addr.ToString() << endl;
    cout << "pubkey ID: " << HexStr(pubkey.GetID()) << endl;
    cout << "pubKey: " << HexStr(pubkey) << endl;
    cout << "privkey: "
         << CBitcoinSecret(secret, helper.privHelper()).ToString() << endl;

    ECC_Stop();
}


string resultWithRPC(const string &method, const Array &params = Array())
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    const Object reply = client.CallRPC(method, params);

    string resultStr;
    int nRet = result(reply, resultStr);
    if (!nRet)
        return resultStr;
    return "";
}

void testPubkeyToAddrAfterGettingParams()
{
    // from cli, getaddresses true
    CPubKey pubkey(ParseHex("027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b"));
    cout << "pubKey: " << HexStr(pubkey) << endl;

    KeysHelper helper;
    //cout << hex << checksum << endl;
    CBitcoinAddress addr(pubkey.GetID(), helper.addrHelper());

    cout << "address : " << addr.ToString() << endl;
}

int main()
{
    cout << "1. test sha256 of \"This is test message\"" << endl;
    testCalcSHA256();

    cout << "2. test sha256 of file, video1.mp4" << endl;
    hashFromFile("video1.mp4");

    cout << "3. temp test" << endl;
    getinfo();

    cout << "4. create key pairs" << endl;
    createKeyPairs();

    cout << "5. pubkey test" << endl;
    testPubkeyToAddrAfterGettingParams();

    return 0;
}
