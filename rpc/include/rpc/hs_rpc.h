#ifndef HS_RPC_H
#define HS_RPC_H

#include <json_spirit/json_spirit.h>

class RpcClient;

json_spirit::Object getinfo(const RpcClient& client);
json_spirit::Object blockChainParams(const RpcClient& client);
json_spirit::Object listunspent(const RpcClient& client, int minConf = 1, int maxConf = 9999999,
                                const std::vector<std::string>& addresses = std::vector<std::string>{});
json_spirit::Object listunspent(const RpcClient& client, const std::vector<std::string>& addresses);
json_spirit::Object listunspent(const RpcClient& client, const std::string& address);
json_spirit::Object lockunspent(const RpcClient& client, bool unlock, std::string txid, int vout);
json_spirit::Object listlockunspent(const RpcClient& client);
json_spirit::Object liststreams(const RpcClient& client, const std::vector<std::string> &streamNames);
json_spirit::Object liststreams(const RpcClient& client, const std::string &streamName = "all");
json_spirit::Object listassets(const RpcClient& client, const std::vector<std::string> &assetNames);
json_spirit::Object listassets(const RpcClient& client, const std::string &name = "");
json_spirit::Object verifymessage(const RpcClient& client,
			const std::string &strAddress,
                        const std::string &strSignmessage,
			const std::string &strText);
json_spirit::Object signmessage(const RpcClient& client,
			const std::string &strPrivateKey,
			const std::string &strText);

json_spirit::Object importaddress(const RpcClient& client, const std::string &address, bool rescan = true);
json_spirit::Object importaddress(const RpcClient& client, const std::vector<std::string>& addresses, bool rescan = true);

json_spirit::Object sendrawtx(const RpcClient& client, const std::string& rawTx);

bool rpcResult(const json_spirit::Object& reply, std::string &resultStr);

#endif // HS_RPC_H
