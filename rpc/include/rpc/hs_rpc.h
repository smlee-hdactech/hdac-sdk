
#ifndef HS_RPC_H
#define HS_RPC_H

#include <json_spirit/json_spirit.h>

class RpcClient;

/// RPC 명령어를 통하여 HDAC 네트워크의 기본 정보를 가져 온다.
json_spirit::Object getinfo(const RpcClient& client);

/// RPC 명령어를 통하여 HDAC 네트워크의 파라미터 설정 정보를 가져 온다.
json_spirit::Object blockChainParams(const RpcClient& client);

/// RPC 명령어를 통하여 아직 보내지 못한 transaction 들의 정보를 가져 온다.
json_spirit::Object listunspent(const RpcClient& client, int minConf = 1, int maxConf = 9999999, 
	const std::vector<std::string>& addresses = std::vector<std::string>{});

/// RPC 명령어를 통하여 아직 보내지 못한 transaction 들의 정보를 가져 온다.
json_spirit::Object listunspent(const RpcClient& client, const std::vector<std::string>& addresses);

/// RPC 명령어를 통하여 아직 보내지 못한 transaction 들의 정보를 가져 온다.
json_spirit::Object listunspent(const RpcClient& client, const std::string& address);

/// RPC 명령어를 통하여 아직 보내지 못한 transaction 을 잠금 하거나 또는 잠금 해제 할 수 있게 한다.
json_spirit::Object lockunspent(const RpcClient& client, bool unlock, std::string txid, int vout);

/// RPC 명령어를 통하여 아직 보내지 못한 transaction 들 중에 잠겨 있는 transaction 들의 정보를 가져 온다.
json_spirit::Object listlockunspent(const RpcClient& client);

/// RPC 명령어를 통하여 생성 된 stream 들의 정보를 가져 온다.
json_spirit::Object liststreams(const RpcClient& client, const std::vector<std::string> &streamNames);

/// RPC 명령어를 통하여 생성 된 stream 들의 정보를 가져 온다.
json_spirit::Object liststreams(const RpcClient& client, const std::string &streamName = "all");

/// RPC 명령어를 통하여 정의 된 asset 들의 정보를 가져온다.
json_spirit::Object listassets(const RpcClient& client, const std::vector<std::string> &assetNames);

/// RPC 명령어를 통하여 정의 된 asset 들의 정보를 가져온다.
json_spirit::Object listassets(const RpcClient& client, const std::string &name = "");

/// RPC 명령어를 통하여 sign 된 메시지를 검증 한다.
json_spirit::Object verifymessage(const RpcClient& client,
			const std::string &strAddress,
                        const std::string &strSignmessage,
			const std::string &strText);

/// RPC 명령어를 통하여 개인키를 이용 하여 sign 된 메시지를 만든다.
json_spirit::Object signmessage(const RpcClient& client,
			const std::string &strPrivateKey,
			const std::string &strText);

/// RPC 명령어를 통하여 특정 지갑주소의 정보를 가져온다.
json_spirit::Object importaddress(const RpcClient& client, const std::string &address, bool rescan = true);

/// RPC 명령어를 통하여 특정 지갑주소의 정보를 가져온다.
json_spirit::Object importaddress(const RpcClient& client, const std::vector<std::string>& addresses, bool rescan = true);

/// RPC 명령어를 통하여 createrawtx 를 통하여 만들어진 rawtransaction 을 전송 한다.
json_spirit::Object sendrawtx(const RpcClient& client, const std::string& rawTx);

/// json 형태의 RPC 결과를 string 으로 변환 시켜 준다.
bool rpcResult(const json_spirit::Object& reply, std::string &resultStr);

#endif // HS_RPC_H
