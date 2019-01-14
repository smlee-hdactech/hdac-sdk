#include <rpc/rpcclient.h>
#include <rpc/rpcresult.h>
#include <rpc/rpccaller.h>
#include <rpc/hs_rpc.h>

using namespace json_spirit;
using namespace std;

void ShowResultWithRPC(const string &method, const Array &params)
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    const Object reply = client.CallRPC(method, params);

    string resultStr;
    int nRet = result(reply, resultStr);
    cout << resultStr << endl;
}

string resultWithRPC(const string &method, const Array &params = Array())
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    const Object reply = client.CallRPC(method, params);

    string resultStr;
    int nRet = result(reply, resultStr);
    if (!nRet)
        return resultStr;
    return "";
}


void testImporAddress()
{
    //RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    RpcClient client{"192.168.70.248", 4260, "hdacrpc", "1234", "kcc"};
    string resultStr;
    if (!rpcResult(importaddress(client, "16NwmLBUM2EoPFRyYbEpgvch7nPcSdsx25qfhW"), resultStr)) {
        cerr << "rpc error :"  << endl;

    }

    cout << "result str: " << endl;
}

void testRawTransaction()
{
    RpcClient client{"192.168.70.248", 4260, "hdacrpc", "1234", "kcc"};
    string resultStr;
    string rawTx("0100000001301d132dfe76e9ebf3c65f4016f8eedd7790bdcf0caf916"
                 "78a6655b5330897ca010000006a47304402204e1ad0881ff3711ff2e9"
                 "28dcc16602800db89aed5551f24b6b432bd444132c5302203cc2df8c6"
                 "64f5dc0da3c33cf61875ecb13e224eabaed7e0522cc5b9284e42d9101"
                 "21039d54d4adeb9f7910e8792b96bc46c9246c99cccc7522f9417d96d"
                 "69b06385d51ffffffff020000000000000000331473706b65171b77de"
                 "7a2e473893d27ad021102430750b73706b6b746573744b6579756a0e7"
                 "465737420666f722064656d6f2e00000000000000001976a91427cb43"
                 "aa429c5b27c62d6a4781645a73ff46d33e88ac00000000");
    if (!rpcResult(sendrawtx(client, rawTx), resultStr)) {
        cerr << "rpc error :"  << endl;

    }

    cout << "result str: " << resultStr << endl;
}

int main()
{
    testRawTransaction();

    testImporAddress();

    string result = resultWithRPC("getblockchainparams");

    std::vector<string> keys{
        "address-pubkeyhash-version",
        "address-scripthash-version",
        "address-checksum-value"
    };
    auto resultItems = findItemsFromRpcResult(result, keys);
    for (const string & item : resultItems) {
        cout << item << endl;
    }

    auto resultMap = mapFromRpcResult(result, keys);
    for (const string & key : keys) {
        cout << key << ": " << resultMap[key] << endl;
    }
    return 0;
}
