#include <iostream>
#include <rpc/rpccaller.h>
#include <structs/hashes.h>
#include <rpc/cif_rpccall.h>
#include <keys/key.h>
#include <utils/utilstrencodings.h>
#include <keys/bitcoinaddress.h>

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

    ShowResultWithRPC("getblockchainparams", Array());

    CKey secret;
    //secret.MakeNewKey(fCompressed);
    secret.MakeNewKey(true);

    CPubKey pubkey = secret.GetPubKey();

    //"address-pubkeyhash-version" : "003fd61c",
    //"address-scripthash-version" : "0571a3e6",
    CBitcoinAddress addr(pubkey.GetID(),
        ParseHex("003fd61c"), ParseHex("0571a3e6"), 0);

    cout << "address : " << addr.ToString() << endl;
    cout << "pubkey ID: " << HexStr(pubkey.GetID()) << endl;
    cout << "pubKey: " << HexStr(pubkey) << endl;

    ECC_Stop();
}

int32_t parseHexToInt32(const string& hexString)
{
    auto hexList = ParseHex(hexString);
    int32_t checksum = 0;
    for (int i = 0; i < hexList.size(); i++) {
        checksum |= ((int32_t)hexList[i]) << 8*i;
    }
    return checksum;
}

void testPubkeyToAddr()
{
    CPubKey pubkey(ParseHex("027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b"));
    cout << "pubKey: " << HexStr(pubkey) << endl;

    //cout << hex << checksum << endl;
    CBitcoinAddress addr(pubkey.GetID(),
        ParseHex("003fd61c"), ParseHex("0571a3e6"), parseHexToInt32("cb507245"));

    cout << "address : " << addr.ToString() << endl;
}

int main()
{
#if 0
    cout << "1. test sha256 of \"This is test message\"" << endl;
    testCalcSHA256();

    cout << "2. test sha256 of file, video1.mp4" << endl;
    hashFromFile("video1.mp4");

    cout << "3. test fo rpc call" << endl;
    Array params;
    ShowResultWithRPC("getinfo", params);

    cout << "4. temp test" << endl;
    getinfo();

    cout << "5. create key pairs" << endl;
    createKeyPairs();

#endif

    cout << "6. pubkey test" << endl;
    testPubkeyToAddr();

    return 0;
}
