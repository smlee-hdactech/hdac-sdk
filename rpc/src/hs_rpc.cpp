#include "hs_rpc.h"
#include "rpcclient.h"
#include <vector>
#include <string>
#include "rpccaller.h"

using namespace std;
using namespace json_spirit;

Object getinfo(const RpcClient& client)
{
    const bool fWait = false; // TODO : to parameter

    return client.CallRPC("getinfo");
}

Object blockChainParams(const RpcClient& client)
{
    //RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    return client.CallRPC("getblockchainparams");
}

Object listunspent(const RpcClient& client, int minConf, int maxConf, const vector<string>& addresses)
{
    //RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    Array params;

    if (minConf > 1) {
        params.push_back(minConf);
    }
    if (maxConf < 9999999)  {
        params.clear();
        params.push_back(minConf);
        params.push_back(maxConf);
    }
    if (!addresses.empty()) {
        params.clear();
        params.push_back(minConf);
        params.push_back(maxConf);
        Array addressArray;
        for (const string& addr : addresses)    {
            addressArray.push_back(addr);
        }
        params.push_back(addressArray);
    }
    return client.CallRPC("listunspent", params);
}

Object listunspent(const RpcClient& client, const vector<string>& addresses)
{
    return listunspent(client, 1, 9999999, addresses);
}

Object listunspent(const RpcClient& client, const string& address)
{
    return listunspent(client, 1, 9999999, vector<string>{address});
}

Object lockunspent(const RpcClient& client, bool unlock, string txid, int vout)
{
    //RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    Array params;
    params.push_back(unlock);
    Array transactions;
    Object transaction;
    transaction.push_back(Pair("txid", txid));
    transaction.push_back(Pair("vout", vout));
    transactions.push_back(transaction);
    params.push_back(transactions);
    return client.CallRPC("lockunspent", params);
}

Object listlockunspent(const RpcClient& client)
{
    //RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    return client.CallRPC("listlockunspent");
}

Object liststreams(const RpcClient& client, const vector<string> &streamNames)
{
    //RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    Array params;
    Array streamArray;

    for (const string& stream : streamNames) {
        streamArray.push_back(stream);
    }
    params.push_back(streamArray);
    return client.CallRPC("liststreams", params);
}

Object liststreams(const RpcClient& client, const string &streamName)
{
    vector<string> streams{streamName};
    return liststreams(client, streams);
}

Object listassets(const RpcClient& client, const vector<string> &assetNames)
{
    Array params;
    Array streamArray;

    for (const string& name : assetNames) {
        streamArray.push_back(name);
    }
    params.push_back(streamArray);
    return client.CallRPC("listassets", params);
}

Object listassets(const RpcClient& client, const string &name)
{
    vector<string> names;
    if (name != "") {
        names.push_back(name);
    }
    return listassets(client, names);
}

bool rpcResult(const Object& reply, string &resultStr)
{
    int nRet = result(reply, resultStr);
    if (nRet) {
        cerr << "rpc error : " << resultStr;
        return false;
    }
    return true;
}
