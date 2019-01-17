#include <iostream>
#include <utils/utilstrencodings.h>
#include <structs/hs_structs.h>
#include <keys/keyshelper.h>
#include <keys/hs_keys.h>
#include <rpc/rpcresult.h>
#include <rpc/rpcclient.h>
#include <rpc/hs_rpc.h>
#include <helpers/hs_helpers.h>

#include <json_spirit/json_spirit_reader_template.h>
#include <json_spirit/json_spirit_writer_template.h>

using namespace std;
using namespace json_spirit;

void testCalcSHA256()
{
    vector<unsigned char> result = obtainHash("This is test message");
    string resultStr = HexStr(result);
    std::transform(resultStr.begin(), resultStr.end(), resultStr.begin(), ::toupper);
    if (resultStr == "DDDBDC2845C9D80DC288710D9B2CF2D6C4F613D0DC4C048A9EA0E8674C2C5E73")   {
        cout << "SHA256 is correct, result: " << resultStr << endl;
    }
    cout << "SHA256 is incorrect, check the program: " << resultStr << endl;
}

// TODO : should add the logging function
// TODO : parameters from config-file.

void testCreateKeyPairs()
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    KeysHelperWithRpc helper(client);

    auto keyPairs = createKeyPairs(helper.privHelper(), helper.addrHelper());
    cout << "address : " << keyPairs.walletAddr << endl;
    cout << "pubkey ID: " << keyPairs.pubkeyHash << endl;
    cout << "pubKey: " << keyPairs.pubkey << endl;
    cout << "privkey: " << keyPairs.privateKey << endl;
}


void testPubkeyToAddrAfterGettingParams()
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    KeysHelperWithRpc helper(client);

    // from cli, getaddresses true
    string pubKey = "027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b";
    cout << "pubKey: " << pubKey << endl;

    //cout << hex << checksum << endl;
    string walletAddr = walletAddrFromPubKey(pubKey, helper.addrHelper());
    cout << "address : " << walletAddr << endl;
}


void testgetinfo()
{
    string resultStr;
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    if (!rpcResult(getinfo(client), resultStr))  {
        return;
    }
    cout << "getinfo: " << resultStr << endl;
}

void testRawTransactionForStreamPublish()
{
    vector<tuple<string, string>> addrNPrivates = {
        make_tuple("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5", "V6X4NaaDQSTgXdAcCzUrSxWqAuFcd53TRXRqmSafUYEbY5DgGMitPEzk"),
        make_tuple("18wD7MBodeTYRAvN5bRuWYB11jwHdkGVCBLSnB", "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp"),
        make_tuple("1EpVCtEHe61hVdgQLKSzM8ZyeFJGdsey29sMQi", "V9ugoEazm16SKbvj7DVxMUcXQnvKpBPaeZ3KEUxTUWoChXQTuHKyzbKx")
    };
    int selected = 0;
    string walletAddr;
    string privateKey;
    tie(walletAddr, privateKey) = addrNPrivates[selected];

    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    string resultStr;
    if (!rpcResult(listunspent(client, walletAddr), resultStr))  {
        return;
    }

    Value resultValue;
    string txid;
    int vout;
    string scriptPubKey;

    cout << "listunspent result: " << endl;
    if (read_string(resultStr, resultValue))   {
        if (resultValue.type() != array_type) {
            throw "wrong result format";
        }
        Array unspents = resultValue.get_array();
        Object selected;
        for (int i = 0; i < unspents.size(); i++) {
            Value value = find_value(unspents[i].get_obj(), "assets");
            if (value.type() == array_type && value.get_array().size() == 0) {
                selected = unspents[i].get_obj();
                break;
            }
        }

        txid = find_value(selected, "txid").get_str();
        vout = find_value(selected, "vout").get_int();
        scriptPubKey = find_value(selected, "scriptPubKey").get_str();
        cout << "txid = " << txid << endl;
        cout << "vout = " << vout << endl;
        cout << "scriptPubKey = " << scriptPubKey << endl;
    }

    if (!rpcResult(lockunspent(client, false, txid, vout), resultStr))  {
        return;
    }

    if (!rpcResult(listlockunspent(client), resultStr))  {
        return;
    }

    //Value resultValue;
    //string txid;
    //int vout;
    cout << "listlockunspent result: " << endl;
    if (read_string(resultStr, resultValue))   {
        if (resultValue.type() != array_type) {
            throw "wrong result format";
        }
        Array unspents = resultValue.get_array();
        txid = find_value(unspents[0].get_obj(), "txid").get_str();
        vout = find_value(unspents[0].get_obj(), "vout").get_int();
        cout << "txid = " << txid << endl;
        cout << "vout = " << vout << endl;
    }

    if (!rpcResult(lockunspent(client, true, txid, vout), resultStr))  {
        return;
    }

    if (!rpcResult(liststreams(client, "stream9"), resultStr))  {
        return;
    }

    string createTxid;
    if (read_string(resultStr, resultValue))   {
        if (resultValue.type() != array_type) {
            throw "wrong result format";
        }
        Array unspents = resultValue.get_array();
        createTxid = find_value(unspents[0].get_obj(), "createtxid").get_str();
        cout << "createtxid = " << createTxid << endl;
    }

    string txHex = createStreamPublishTx("key1", "first programmed version", createTxid,
                                         scriptPubKey, txid, vout, "", privateKey, KeysHelperWithRpc(client).privHelper());
    cout << "raw transaction: " << txHex << endl;
}

void testRawTransactionForAssetSend()
{
    vector<tuple<string, string>> addrNPrivates = {
        make_tuple("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5", "V6X4NaaDQSTgXdAcCzUrSxWqAuFcd53TRXRqmSafUYEbY5DgGMitPEzk"),
        make_tuple("18wD7MBodeTYRAvN5bRuWYB11jwHdkGVCBLSnB", "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp"),
        make_tuple("1EpVCtEHe61hVdgQLKSzM8ZyeFJGdsey29sMQi", "V9ugoEazm16SKbvj7DVxMUcXQnvKpBPaeZ3KEUxTUWoChXQTuHKyzbKx")
    };

    int selected = 1;
    string walletAddr;
    string privateKey;
    tie(walletAddr, privateKey) = addrNPrivates[selected];

    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    string resultStr;
    if (!rpcResult(listunspent(client, walletAddr), resultStr))  {
        return;
    }

    Value resultValue;
    string txid;
    int vout;
    string scriptPubKey;
    double unspentQty;

    string assetName = "ass1";
    cout << "listunspent result: " << endl;
    if (read_string(resultStr, resultValue))   {
        if (resultValue.type() != array_type) {
            throw "wrong result format";
        }

        Array unspents = resultValue.get_array();
        Object selected;
        for (int i = 0; i < unspents.size(); i++) {
            Value value = find_value(unspents[i].get_obj(), "assets");
            if (value.type() == array_type && value.get_array().size() != 0) {
                bool found = false;
                Array assetInfo = value.get_array();
                for (int i = 0; i < assetInfo.size(); i++) {
                //for (const Object& assetInfo : value.get_array())   {
                    if (assetInfo[i].type() == obj_type)    {
                        Value name = find_value(assetInfo[i].get_obj(), "name");
                        if (name.get_str() == assetName)   {
                            found = true;
                            Value qty = find_value(assetInfo[i].get_obj(), "qty");
                            unspentQty = qty.get_real();
                            break;
                        }
                    }
                }
                if (found == true)  {
                    selected = unspents[i].get_obj();
                    break;
                }
            }
        }

        txid = find_value(selected, "txid").get_str();
        vout = find_value(selected, "vout").get_int();
        scriptPubKey = find_value(selected, "scriptPubKey").get_str();
        cout << "txid = " << txid << endl;
        cout << "vout = " << vout << endl;
        cout << "scriptPubKey = " << scriptPubKey << endl;
    }

    if (!rpcResult(lockunspent(client, false, txid, vout), resultStr))  {
        return;
    }

    if (!rpcResult(listlockunspent(client), resultStr))  {
        return;
    }

    //Value resultValue;
    //string txid;
    //int vout;
    cout << "listlockunspent result: " << endl;
    if (read_string(resultStr, resultValue))   {
        if (resultValue.type() != array_type) {
            throw "wrong result format";
        }
        Array unspents = resultValue.get_array();
        txid = find_value(unspents[0].get_obj(), "txid").get_str();
        vout = find_value(unspents[0].get_obj(), "vout").get_int();
        cout << "txid = " << txid << endl;
        cout << "vout = " << vout << endl;
    }

    if (!rpcResult(lockunspent(client, true, txid, vout), resultStr))  {
        return;
    }

    if (!rpcResult(listassets(client, assetName), resultStr))  {
        return;
    }

    string issueTxid;
    int multiple;
    if (read_string(resultStr, resultValue))   {
        if (resultValue.type() != array_type) {
            throw "wrong result format";
        }
        Array assets = resultValue.get_array();
        issueTxid = find_value(assets[0].get_obj(), "issuetxid").get_str();
        cout << "issuetxid = " << issueTxid << endl;
        multiple = find_value(assets[0].get_obj(), "multiple").get_int();
        cout << "multiple = " << multiple << endl;
    }

    string txHex = createAssetSendTx("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5",
                10, issueTxid, multiple,
                scriptPubKey, txid, vout,
                unspentQty, "", privateKey,
                KeysHelperWithRpc(client).privHelper(),
                KeysHelperWithRpc(client).addrHelper());
    cout << "raw transaction: " << txHex << endl;
}

void testHashFromFile()
{
    auto result = hashFromFile("video1.mp4");
    cout << "file hash: " << HexStr(result) << endl;
}

int main()
{
    cout << "1. test sha256 of \"This is test message\"" << endl;
    testCalcSHA256();

    cout << "2. test sha256 of file, video1.mp4" << endl;
    testHashFromFile();

    cout << "3. test getinfo" << endl;
    testgetinfo();

    cout << "4. create key pairs" << endl;
    testCreateKeyPairs();

    cout << "5. pubkey test" << endl;
    testPubkeyToAddrAfterGettingParams();

    cout << "6. stream publish test" << endl;
    try {
        testRawTransactionForStreamPublish();
    } catch(std::exception &e) {
        cout << e.what() << endl;
    }

    cout << "7. asset send test" << endl;
    try {
        testRawTransactionForAssetSend();
    } catch(std::exception &e) {
        cout << e.what() << endl;
    }

    return 0;
}
