#include <iostream>
#include <rpc/rpccaller.h>
#include <structs/hashes.h>
#include <rpc/cif_rpccall.h>
#include <keys/key.h>
#include <utils/strcodeclib.h>

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

    CKey secret;
    //secret.MakeNewKey(fCompressed);
    secret.MakeNewKey(true);

    CPubKey pubkey = secret.GetPubKey();

    cout << "pubkey ID: " << HexStr(pubkey.GetID()) << endl;
    cout << "pubKey: " << HexStr(pubkey) << endl;

    ECC_Stop();
}

int main()
{
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

    return 0;
}
