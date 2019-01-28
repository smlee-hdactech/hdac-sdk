/**
* @file		hs_keys_wrapper.cpp
* @date		2019-01-25
* @author	HDAC Technology Inc.
*
* @brief	hs_keys_wrapper 소스 파일.
*/

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include "hs_keys.h"
#include "keyshelper.h"
#include <utils/utilstrencodings.h>
#include "hs_keys_wrapper.h"
#ifdef _WIN32
#include <combaseapi.h>
#else
#include <string.h>
#endif

using namespace std;

struct PrivateKeyHelperConstant : public IPrivateKeyHelper {
        PrivateKeyHelperConstant(const char* privateKeyPrefix, const char* addrChecksum) :
                _privatekeyPrefix(ParseHex(privateKeyPrefix)) {
                _addrChecksumValue = parseHexToInt32Le(addrChecksum);
                }

        const std::vector<unsigned char> privkeyPrefix() const override {
                return _privatekeyPrefix;
        }

        int32_t addrChecksumValue() const override {
                return _addrChecksumValue;
        }

        vector<unsigned char> _privatekeyPrefix;
        int32_t _addrChecksumValue;
};

struct WalletAddrHelperConstant : public IWalletAddrHelper {
        WalletAddrHelperConstant(const char* pubkeyAddrPrefix, const char* scriptAddrPrefix, const char* addrChecksum) :
                _pubKeyAddrPrefix(ParseHex(pubkeyAddrPrefix)),
                _scriptAddrPrefix(ParseHex(scriptAddrPrefix)) {
                _addrChecksumValue = parseHexToInt32Le(addrChecksum);
                }

        const std::vector<unsigned char> pubkeyAddrPrefix() const override {
                return _pubKeyAddrPrefix;
        }

        const std::vector<unsigned char> scriptAddrPrefix() const override {
                return _scriptAddrPrefix;
        }

        int32_t addrChecksumValue() const override {
                return _addrChecksumValue;
        }

        vector<unsigned char> _pubKeyAddrPrefix;
        vector<unsigned char> _scriptAddrPrefix;
        int32_t _addrChecksumValue;
};

#ifdef _WIN32
char * create_stream_publish_tx_shp(const char* streamKey, const char* streamItem, const char* createTxid,
	const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
	const char* unspentRedeemScript, const char* privateKey, struct PrivateKeyHelpInfo *helper) {

	char * strTx = create_stream_publish_tx(streamKey, streamItem, createTxid, unspentScriptPubKey, unspentTxid, 
		unspentVOut, unspentRedeemScript, privateKey, helper);
	unsigned long ulSize = strlen(strTx) + sizeof(char);
	char* pszReturn = NULL;

	pszReturn = (char*)::CoTaskMemAlloc(ulSize);
	// Copy the contents of szSampleString
	// to the memory pointed to by pszReturn.
	strcpy_s(pszReturn, ulSize, strTx);

	delete[] strTx;
	// Return pszReturn.
	return pszReturn;
}
#endif

/**
 *
 * @brief createStreamPublishTx 함수를 c에서 사용하기 위해 wrapping 한 함수. 
 * @details 스트림키 발행을 위한 raw-tx 문자열을 생성하는 createStreamPublishTx 함수를 c에서 사용하기 위해 wrapping 함 함수.
 * @param streamKey     스트림에 대한 키
 * @param streamItem    키에 대한 값
 * @param createTxid            스트림 생성 트랜잭션 ID
 * @param unspentScriptPubKey   UTXO에 대한 scriptPubKey
 * @param unspentTxid                   UTXO에 대한 트랜잭션 ID
 * @param unspentVOut                   UTXO의 인덱스
 * @param unspentRedeemScript   UTXO의 redeem script (muti-sig 용으로 주로 사용)
 * @param privateKey                    보내는 지갑에 대한 개인키
 * @param helper                 개인키 처리를 위한 정보 제공 인터페이스
 *
 * @return raw-tx 문자열
 *
 */
char * create_stream_publish_tx(const char* streamKey, const char* streamItem, const char* createTxid,
	const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
	const char* unspentRedeemScript, const char* privateKey, struct PrivateKeyHelpInfo *helper) {

	PrivateKeyHelperConstant privHelper(helper->privateKeyPrefix, helper->addrChecksum);

	string result = createStreamPublishTx(streamKey, streamItem, createTxid, unspentScriptPubKey, unspentTxid,
		unspentVOut, unspentRedeemScript, privateKey, privHelper);
	char *retVal = static_cast<char*>(malloc(result.size() + 1));

#ifdef _WIN32	
	strcpy_s(retVal, result.size()+1, result.c_str());
#else
	strcpy(retVal, result.c_str());
#endif	

	return retVal;
}

/**
 *
 * @brief createKeyPairs 함수를 c에서 사용하기 위해 wrapping 한 함수.
 * @details 개인키를 생성하는 createKeyPairs 함수를 c에서 사용 하기 위해 wrapping 한 함수.
 * @param privatehelper 개인키 처리를 위한 정보 제공 인터페이스
 * @param addrhelper 지갑주소 처리를 위한 정보 제공 인터페이스
 *
 * @return KeyPairs
 *
 */
keypairs_type_t *create_key_pairs(struct PrivateKeyHelpInfo *privatehelper,
			  		struct WalletAddrHelpInfo *addrhelper)
{
	PrivateKeyHelperConstant privHelper(privatehelper->privateKeyPrefix, privatehelper->addrChecksum);
	WalletAddrHelperConstant addrHelper(addrhelper->pubKeyAddrPrefix, addrhelper->scriptAddrPrefix, addrhelper->addrChecksum);

	auto keyPairs = createKeyPairs(privHelper, addrHelper);

	keypairs_type_t *keys = static_cast<keypairs_type_t*>(malloc(sizeof(keypairs_type_t)));

#ifdef _WIN32	
	strcpy_s(keys->privatekey, sizeof(keys->privatekey), keyPairs.privateKey.c_str());
	strcpy_s(keys->pubkey, sizeof(keys->pubkey), keyPairs.pubkey.c_str());
	strcpy_s(keys->pubkeyhash, sizeof(keys->pubkeyhash), keyPairs.pubkeyHash.c_str());
	strcpy_s(keys->walletaddr, sizeof(keys->walletaddr), keyPairs.walletAddr.c_str());
#else
	strcpy(keys->privatekey, keyPairs.privateKey.c_str());
	strcpy(keys->pubkey, keyPairs.pubkey.c_str());
	strcpy(keys->pubkeyhash, keyPairs.pubkeyHash.c_str());
	strcpy(keys->walletaddr, keyPairs.walletAddr.c_str());
#endif	
	
	return keys;
}

/**
 *
 * @brief createAssetSendTx 함수를 c에서 사용 하기 위해 wrapping 한 함수
 * @detauls 개인키 처리를 위한 정보 제공 인터페이스를 가져오는 createAssetSendTx 함수를 c에섯 사용 하기 위해 wrapping 한 함수.
 * @param toAddr        보낼 지갑 주소
 * @param quantity      보낼 자산량
 * @param issueTxid             발행한 트랜잭션 ID
 * @param multiple              자산에 대한 multiple 값
 * @param unspentScriptPubKey   UTXO에 대한 scriptPubKey
 * @param unspentTxid                   UTXO에 대한 트랜잭션 ID
 * @param unspentVOut                   UTXO의 인덱스
 * @param unspentQty                    UTXO의 양
 * @param unspentRedeemScript   UTXO의 redeem script (muti-sig 용으로 주로 사용)
 * @param privateKey                    보내는 지갑에 대한 개인키
 * @param privatehelper                 개인키 처리를 위한 정보 제공 인터페이스
 * @param addrhelper                  지갑주소 처리를 위한 정보 제공 인터페이스
 *
 * @return 개인키 처리를 위한 정보 제공 인터페이스
 *
 */
char *create_asset_send_tx(const char *toAddr, double quantity, const char *issueTxid, int multiple,
			const char *unspentScriptPubKey, const char *unspentTxid, uint32_t unspentVout,
			double unspentQty, const char * unspentRedeemScript, const char *privateKey,
			struct PrivateKeyHelpInfo *privatehelper, struct WalletAddrHelpInfo *addrhelper)
{
	PrivateKeyHelperConstant privHelper(privatehelper->privateKeyPrefix, privatehelper->addrChecksum);
	WalletAddrHelperConstant addrHelper(addrhelper->pubKeyAddrPrefix, addrhelper->scriptAddrPrefix, addrhelper->addrChecksum);

	string result = createAssetSendTx(toAddr, quantity, issueTxid, multiple, unspentScriptPubKey, unspentTxid, unspentVout,
				unspentQty, unspentRedeemScript, privateKey, privHelper, addrHelper);

	char *retVal = static_cast<char*>(malloc(result.size() + 1));

#ifdef _WIN32	
	strcpy_s(retVal, result.size()+1, result.c_str());
#else
	strcpy(retVal, result.c_str());
#endif	

	return retVal;
}

/**
 *
 * @brief SignMessage 함수를 c에서 사용 하기 위해 wrapping 한 함수.
 * @details 개인키를 이용하여 sign 된 메시지를 만드는 SignMessage 함수를 c에서 사용 하기 위해 wrapping 한 함수.
 * @details 개인키를 이용하여 평문으로 된 메시지를 sign 하여 암호화 한다.
 * @param strAddress sign 할려고 하는 개인키 값
 * @param strMessage sign 할려고 하는 원본 문자열
 * @param privateHelper 개인키 처리를 위한 정보 제공 인터페이스
 * @param addrHelper 지갑주소 처리를 위한 정보 제공 인터페이스
 *
 * @return base64 로 인코딩 된 sign 된 문자열
 *
 */
char *sign_message(const char *strAddress, const char *strMessage,
		struct PrivateKeyHelpInfo *privatehelper, struct WalletAddrHelpInfo *addrhelper)
{
	PrivateKeyHelperConstant privHelper(privatehelper->privateKeyPrefix, privatehelper->addrChecksum);
	WalletAddrHelperConstant addrHelper(addrhelper->pubKeyAddrPrefix, addrhelper->scriptAddrPrefix, addrhelper->addrChecksum);

	string result = SignMessage(strAddress, strMessage, privHelper, addrHelper);

	char *retVal = static_cast<char*>(malloc(result.size() + 1));

#ifdef _WIN32	
	strcpy_s(retVal, result.size()+1, result.c_str());
#else
	strcpy(retVal, result.c_str());
#endif	

	return retVal;
}


/**
 *
 * @brief VerifyMessage 함수를 c에서 사용 하기 위해 wrapping 한 함수.
 * @details 개인키 또는 지갑주소로 sign 된 메시지를 검증 하는 VerifyMessage 함수를 c에서 사용 하기 위해 wrapping 한 함수.
 * @param strAddress sign 할 때 사용 된 지갑주소
 * @param strSign sign 되어진 문자열
 * @param strMessage 원본 문자열
 * @param addrhelper 지갑주소 처리를 위한 정보 제공 인터페이스
 *
 * @return 해당 개인키 또는 지갑주소로 sign 메시지가 맞다면 true 아니라면 false
 *
 */
int verify_message(const char *strAddress, const char *strSign, const char *strMessage, struct WalletAddrHelpInfo *addrhelper)
{
	WalletAddrHelperConstant addrHelper(addrhelper->pubKeyAddrPrefix, addrhelper->scriptAddrPrefix, addrhelper->addrChecksum);

	int result = VerifyMessage(strAddress, strSign, strMessage, addrHelper);

	return result;
}

#ifdef _WIN32

void test_return_mashal(struct TestStruct * retVal)
{
	//struct TestStruct* retVal;

	//retVal = (struct TestStruct*)::CoTaskMemAlloc(sizeof(struct TestStruct));

	sprintf_s(retVal->stringData1, sizeof(retVal->stringData1), "just test stringData1");
	retVal->ulongData = 100;
	sprintf_s(retVal->stringData2, sizeof(retVal->stringData2), "just test stringData2");
	retVal->intData = 500;

	//return retVal;
}
#endif