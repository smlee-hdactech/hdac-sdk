#include "KeysHelperWithRpc.h"
#include <json_spirit/json_spirit.h>
#include <utils/utilstrencodings.h>
#include <rpc/rpcresult.h>
#include <rpc/hs_rpc.h>

using namespace std;
using namespace json_spirit;

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

IWalletAddrHelper &KeysHelperWithRpc::addrHelper() {
    return *_addrHelper;
}

IPrivateKeyHelper &KeysHelperWithRpc::privHelper() {
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
