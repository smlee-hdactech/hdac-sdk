/**
* @file		KeysHelperWithRpc.cpp
* @date		2019-01-17
* @author	HDAC Technology Inc.
*
* @brief	hs_helpers 소스 파일 .
*/

#include "KeysHelperWithRpc.h"
#include <json_spirit/json_spirit.h>
#include <utils/utilstrencodings.h>
#include <rpc/rpcresult.h>
#include <rpc/hs_rpc.h>

using namespace std;
using namespace json_spirit;

/**
 *
 * @brief 개인키와 지갑주소를 생성하기 위한 정보를 제공 한다.
 * @details 개인키와 지갑주소를 생성하기 위한 정보를 HDAC 네트워크와
 * RPC 통신을 해서 가져온다.
 * @param const RpcClient & client HDAC 네트워크 접속 정보를 제공한다.
 *
 * @return 없음
 *
 */
KeysHelperWithRpc::KeysHelperWithRpc(const RpcClient &client) {
    string resultStr;

    if (!rpcResult(blockChainParams(client), resultStr)) {
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

/**
 *
 * @brief 지갑주소 처리를 위한 정보 제공 인터페이스를 가져온다.
 * @details 주로 지갑주소 생성을 위해 내부적으로 사용 된다.
 *
 * @return 지갑주소 처리를 위한 정보 제공 인터페이스
 *
 */
const IWalletAddrHelper &KeysHelperWithRpc::addrHelper() const {
    return *_addrHelper;
}

/**
 *
 * @brief 개인키 처리를 위한 정보 제공 인터페이스를 가져온다.
 * @details 주로 개인키 처리를 위해 내부적으로 사용 된다.
 *
 * @return 개인키 처리를 위한 정보 제공 인터페이스
 *
 */
const IPrivateKeyHelper &KeysHelperWithRpc::privHelper() const {
    return *_privHelper;
}

const std::vector<unsigned char> KeysHelperWithRpc::WalletAddrHelper::pubkeyAddrPrefix() const  {
    return ParseHex(_resultMap.at("address-pubkeyhash-version"));
}

const std::vector<unsigned char> KeysHelperWithRpc::WalletAddrHelper::scriptAddrPrefix() const  {
    return ParseHex(_resultMap.at("address-scripthash-version"));
}

int32_t KeysHelperWithRpc::WalletAddrHelper::addrChecksumValue() const {
    return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
}

const std::vector<unsigned char> KeysHelperWithRpc::PrivateKeyHelper::privkeyPrefix() const {
    return ParseHex(_resultMap.at("private-key-version"));
}

int32_t KeysHelperWithRpc::PrivateKeyHelper::addrChecksumValue() const  {
    return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
}
