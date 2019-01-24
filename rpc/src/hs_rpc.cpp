/**
* @file		hs_rpc.cpp
* @date		2019-01-17
* @author	HDAC Technology Inc.
*
* @brief	hs_rpc 소스 코드 파일.
*/

#include "hs_rpc.h"
#include "rpcclient.h"
#include <vector>
#include <string>
#include "rpccaller.h"

using namespace std;
using namespace json_spirit;

/**
 *
 * @brief RPC 명령어를 통하여 createrawtx 를 통하여 만들어진
 * rawtransaction 을 전송 한다. 
 * @details createrawtx 명령어를 통하여 sign 된 16 진수 Hex
 * rawtransaction 을 전송 한다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param rawTx createrawtx 명령어를 통하여 만들어진 sign 된 16진수 Hex rawtransaction 문자열
 *
 * @return sendrawtx RPC 명령의 결과 값, transaction hash 값
 *
 */
Object sendrawtx(const RpcClient& client, const string& rawTx)
{
    Array params;
    params.push_back(rawTx);
    return client.CallRPC("sendrawtx", params);
}


/**
 *
 * @brief RPC 명령어를 통하여 특정 주소의 정보를 가져 온다. 
 * @details rescan 을 통하여 특정 주소의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param address 정보를 가져올 특정 주소 문자열
 * @param rescan 특정 지갑주소의 정보를 가져올 때, rescan을 수행 할 것인지 아닌지에 대한 여부 (default = true)
 *
 * @return importaddress RPC 명령어의 결과 값, (없음)
 *
 */
Object importaddress(const RpcClient& client, const string &address, bool rescan)
{
    vector<string> addresses;
    addresses.push_back(address);
    return importaddress(client, addresses, rescan);
}

/**
 *
 * @brief RPC 명령어를 통하여 특정 주소의 정보를 가져 온다. 
 * @details rescan 을 통하여 특정 주소의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param addresses 정보를 가져올 특정 주소 문자열
 * @param rescan 특정 지갑주소의 정보를 가져올 때, rescan을 수행 할 것인지 아닌지에 대한 여부 (default = true)
 *
 * @return importaddress RPC 명령어의 결과 값, (없음)
 *
 */
Object importaddress(const RpcClient& client, const vector<string>& addresses, bool rescan)
{
    Array params;
    Array addressArray;
    for(const string &addr : addresses) {
        addressArray.push_back(addr);
    }
    params.push_back(addressArray);
    params.push_back("");
    params.push_back(rescan);
    return client.CallRPC("importaddress", params);
}

/**
 * @fn Object getinfo(const RpcClient& client)
 * @brief RPC 명령어를 통하여 HDAC 네트워크의 기본 정보를 가져 온다. 
 * @details HDAC 네트워크의 버전, 블럭 높이, 체인 이름 등의 기본 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 *
 * @return getinfo RPC 명령어의 결과 값, (버전, 블럭 높이,
 * 체인 이름등의 HDAC 네트워크의 기본 정보)
 *
 */
Object getinfo(const RpcClient& client)
{
    const bool fWait = false; // TODO : to parameter

    return client.CallRPC("getinfo");
}

/**
 * @fn Object blockChainParams(const RpcClient& client)
 * @brief RPC 명령어를 통하여 HDAC 네트워크의 파라미터 설정 정보를 가져 온다. 
 * @details HDAC 네트워크의 지갑 주소 체크섬, 체인 이름, rpc port 등의 파라미터 설정 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 *
 * @return blockchainparams RPC 명령어의 결과 값, (지갑 주소 체크섬, 체인 이름, rpc port 등의 HDAC 네트워크의 파라미터 설정 정보)
 *
 */
Object blockChainParams(const RpcClient& client)
{
    return client.CallRPC("getblockchainparams");
}

/**
 * @fn Object listunspent(const RpcClient& client, int minConf, int maxConf, const vector<string>& addresses)
 * @brief RPC 명령어를 통하여 아직 보내지 못한 transaction 들의 정보를 가져 온다. 
 * @details HDAC 네트워크의 지갑 주소 체크섬, 체인 이름, rpc port 등의 파라미터 설정 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param minConf 최소 컨펌 수(filter 역할, 범위 시작)
 * @param maxConf 최대 컨펌 수(filter 역할, 범위 끝)
 * @param addresses 조회하고자 하는 특정 지갑 주소들
 *
 * @return listunspent RPC 명령어의 결과 값, (특정 지갑에 보내지 못한 transaction 리스트)
 *
 */
Object listunspent(const RpcClient& client, int minConf, int maxConf, const vector<string>& addresses)
{
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

/**
 * @fn Object listunspent(const RpcClient& client, const vector<string>& addresses)
 * @brief RPC 명령어를 통하여 아직 보내지 못한 transaction 들의 정보를 가져 온다. 
 * @details 특정 지갑주소들에서 생성하여 아직 보내지 못한 transaction 들의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param addresses 조회하고자 하는 특정 지갑 주소들
 *
 * @return listunspent RPC 명령어의 결과 값, (특정 지갑에 보내지 못한 transaction 리스트)
 *
 */
Object listunspent(const RpcClient& client, const vector<string>& addresses)
{
    return listunspent(client, 1, 9999999, addresses);
}

/**
 * @fn Object listunspent(const RpcClient& client, const string& address)
 * @brief RPC 명령어를 통하여 아직 보내지 못한 transaction 들의 정보를 가져 온다. 
 * @details 특정 지갑주소들에서 생성하여 아직 보내지 못한
 * transaction 들의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param address 조회하고자 하는 특정 지갑 주소들
 *
 * @return listunspent RPC 명령어의 결과 값, (특정 지갑에 보내지 못한 transaction 리스트)
 *
 */
Object listunspent(const RpcClient& client, const string& address)
{
    return listunspent(client, 1, 9999999, vector<string>{address});
}

/**
 * @fn Object lockunspent(const RpcClient& client, bool unlock, string txid, int vout)
 * @brief RPC 명령어를 통하여 아직 보내지 못한 transaction 을 잠금 하거나
 * 또는 잠금 해제 할 수 있게 한다.
 * @details 아직 보내지 못한 transaction 에 대하여 보내지 못 하도록 잠금 하거나
 * 또는 잠겨 있는 transaction 의 잠금을 해제 할 수 있게 한다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param unlock 잠금 할 껀지,  잠금 해제 할 껀지  여부
 * @param txid 잠금 또는 잠금 해제 할 transaction 의 ID
 * @param vout 잠금 또는 잠금 해제 하고자 하는 transaction 의 vout 양
 *
 * @return lockunspent RPC 명령어의 결과 값, (잠금 또는 잠금 해제 성공 여부,
 * true, false)
 *
 */
Object lockunspent(const RpcClient& client, bool unlock, string txid, int vout)
{
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

/**
 * @fn Object listlockunspent(const RpcClient& client)
 * @brief RPC 명령어를 통하여 아직 보내지 못한 transaction 중에 잠겨 있는
 * transaction 들의 정보를 가져 온다.
 * @details 아직 보내지 못한 transaction 들 중에 전송 되지 못하도록 잠겨 있는 transaction 들의 정버를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 *
 * @return listlockunspent RPC 명령어의 결과 값, (잠금 되어진 transaction 들의 ID 와 vout 값)
 *
 */
Object listlockunspent(const RpcClient& client)
{
    return client.CallRPC("listlockunspent");
}

/**
 *
 * @brief RPC 명령어를 통하여 생성 된 stream 들의 정보를 가져 온다.
 * @details 생성 되어진 stream 들의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param streamNames 특정 stream 의 정보를 가져올 때 사용 한다.
 *
 * @return liststreams RPC 명령어의 결과 값, (생성되어진 stream 정보)
 *
 */
Object liststreams(const RpcClient& client, const vector<string> &streamNames)
{
    Array params;
    Array streamArray;

    for (const string& stream : streamNames) {
        streamArray.push_back(stream);
    }
    params.push_back(streamArray);
    return client.CallRPC("liststreams", params);
}

/**
 *
 * @brief RPC 명령어를 통하여 생성 된 stream 들의 정보를 가져 온다.
 * @details 생성 되어진 stream 들의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param streamName 특정 stream 의 정보를 가져 올 때 사용 한다.
 *
 * @return liststreams RPC 명령어의 결과 값, (생성되어진 stream 정보)
 *
 */
Object liststreams(const RpcClient& client, const string &streamName)
{
    vector<string> streams{streamName};
    return liststreams(client, streams);
}

/**
 *
 * @brief RPC 명령어를 통하여 정의 된 asset 들의 정보를 가져 온다.
 * @details 정의 되어진 asset 들의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param assetNames 특정 asset 들의 정보를 가져올 때 사용 한다.
 *
 * @return listassets RPC 명령어의 결과 값, (생성되어진 asset 정보)
 *
 */
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

/**
 *
 * @brief RPC 명령어를 통하여 정의 된 asset 들의 정보를 가져 온다.
 * @details 정의 되어진 asset 들의 정보를 가져 온다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param name 특정 asset 들의 정보를 가져
 * 올 때 사용 한다.
 *
 * @return listassets RPC 명령어의 결과 값, (생성되어진 asset 정보)
 *
 */
Object listassets(const RpcClient& client, const string &name)
{
    vector<string> names;
    if (name != "") {
        names.push_back(name);
    }
    return listassets(client, names);
}

/**
 *
 * @brief RPC 명령어를 통하여 개인키를 이용 하여 sign 된 메시지를 만든다.
 * @details 개인키를 이용하여 평문의 문자열을 sign 된 문자열로 변환 한다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param strPrivateKey sign 할려고하는 개인키 값
 * @param strText sign 할려고하는 평문의 문자열
 *
 * @return signmessage RPC 명령어의 결과 값, (생성되어진 sign 메시지 문자열)
 *
 */
Object signmessage(const RpcClient& client, const string &strPrivateKey,
			const string &strText)
{
    Array params;
    if (strPrivateKey != "") {
        params.push_back(strPrivateKey);
    }
    if (strText != "") {
        params.push_back(strText);
    }

    return client.CallRPC("signmessage", params);
}

/**
 *
 * @brief RPC 명령어를 통하여 sign 된 메시지를 검증 한다.
 * @details signmessage 를 통하여 sign 된 메시지가 제대로 sign 되었는지 검증 한다.
 * @param client HDAC 네트워크 접속 정보를 제공한다.
 * @param strAddress sign 할 때 사용 된 지갑 주소
 * @param strSignmessage sign 되어진 문자열
 * @param strText sign 한 원본 문자열
 *
 * @return verifymessage RPC 명령어의 결과 값, (검증 성공 여부, true, false)
 *
 */
Object verifymessage(const RpcClient& client, const string &strAddress,
			const string &strSignmessage, const string &strText)
{
    Array params;
    if (strAddress != "") {
        params.push_back(strAddress);
    }
    if (strSignmessage != "") {
        params.push_back(strSignmessage);
    }
    if (strText != "") {
        params.push_back(strText);
    }

    return client.CallRPC("verifymessage", params);
}

/**
 *
 * @brief json 형태의 RPC 명령어 결과를 string 형으로 변환 시켜 준다.
 * @details json 형태로 반환되는 RPC 명령어 결과 값들을 string 형으로
 * 변환 시켜 준다.
 * @param reply RPC 명령어로 반환 된 json 형태의 결과 값
 * @param resultStr string 형으로 변환 된 결과 값
 *
 * @return json 형태에서 string 형으로 변환 성공 여부, true, false
 *
 */
bool rpcResult(const Object& reply, string &resultStr)
{
    int nRet = result(reply, resultStr);
    if (nRet) {
        cerr << "rpc error : " << resultStr;
        return false;
    }
    return true;
}
