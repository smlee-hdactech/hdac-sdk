#include <rpc/rpcclient.h>
#include <rpc/rpcresult.h>
#include <rpc/rpccaller.h>

using namespace json_spirit;
using namespace std;

void ShowResultWithRPC(const string &method, const Array &params)
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    const Object reply = client.CallRPC(method, params);

    string resultStr;
    int nRet = result(reply, resultStr);
    cout << resultStr << endl;
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



int main()
{
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
