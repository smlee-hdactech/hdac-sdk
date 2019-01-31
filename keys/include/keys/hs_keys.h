#ifndef HS_KEYS_H
#define HS_KEYS_H

#include <string>
#include <memory>
#include "bitcoinaddress.h"
/**
 *
 * @brief 개인키 정보
 * @details 개인키와 이에 따른 공개키, 공개키 해시, 지갑주소를 담고 있다.
 * @author HDAC Technology Inc.
 * @date 2019-1-17
 * @version 0.0.1
 *
 */
struct KeyPairs {
    std::string privateKey;
    std::string pubkey;
    std::string pubkeyHash;
    std::string walletAddr;
};

class IPrivateKeyHelper;
class IWalletAddrHelper;

/// 개인키를 생성한다.
KeyPairs createKeyPairs(const IPrivateKeyHelper &privateHelper, const IWalletAddrHelper &addrHelper);

class IWalletAddrHelper;
class IPrivateKeyHelper;

/// 스트림키 발행을 위한 raw-tx 문자열을 생성한다.
std::string createStreamPublishTx(const std::string& streamKey, const std::string& streamItem,
		const std::string &createTxid,
		const std::string &unspentScriptPubKey, const std::string &unspentTxid, uint32_t unspentVOut,
		const std::string &unspentRedeemScript, const std::string &privateKey,
		const IPrivateKeyHelper &helper);

/// 개인키 처리를 위한 정보 제공 인터페이스를 가져온다.
std::string createAssetSendTx(const std::string& toAddr, double quantity,
		const std::string& issueTxid, int multiple,
		const std::string& unspentScriptPubKey, const std::string& unspentTxid, uint32_t unspentVOut,
		double unspentQty, const std::string &unspentRedeemScript,
		const std::string& privateKey, const IPrivateKeyHelper& privateHelper, const IWalletAddrHelper &walletHelper);

std::string walletAddrFromPubKey(const std::string& pubkeyStr, const IWalletAddrHelper& addrHelpler);

/// 개인키 또는 지갑주소로 sign 된 메시지를 검증 한다.
bool VerifyMessage(const std::string &strAddress, const std::string &strSign,
		const std::string &strMessage, const IWalletAddrHelper &addrHelper);

/// 개인키를 이용 하여 sign 된 메시지를 만든다.
std::string SignMessage(const std::string &strAddress, const std::string &strMessage, 
		const IPrivateKeyHelper &privateHelper, const IWalletAddrHelper &addrHelper);

#endif // HS_KEYS_H
