#ifndef HS_KEYS_WRAPPER_H
#define HS_KEYS_WRAPPER_H

#if (defined _WIN32) || (defined _WIN64)
#define DECL_EXPORT __declspec(dllexport)
#define DECL_IMPORT __declspec(dllimport)
#else
#define DECL_EXPORT
#define DECL_IMPORT
#endif

#if defined(keys_wrapper_EXPORTS)
#  define keys_wrapper_EXPORT DECL_EXPORT
#else
#  define keys_wrapper_EXPORT DECL_IMPORT
#endif

#include <stdio.h>

typedef struct keypairs {
	char privatekey[100];
	char pubkey[100];
	char pubkeyhash[100];
	char walletaddr[100];
} keypairs_type_t;

struct PrivateKeyHelpInfo {
	char privateKeyPrefix[10];
	char addrChecksum[10];
};

struct WalletAddrHelpInfo {
	char pubKeyAddrPrefix[10];
	char scriptAddrPrefix[10];
	char addrChecksum[10];
};

struct TestStruct {
	char stringData1[100];
	uint64_t ulongData;

	char stringData2[200];
	int32_t intData;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

	keys_wrapper_EXPORT char* create_stream_publish_tx_shp(const char* streamKey, const char* streamItem, const char* createTxid,
		const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
		const char* unspentRedeemScript, const char* privateKey, struct PrivateKeyHelpInfo *helper);

	keys_wrapper_EXPORT void test_return_mashal(struct TestStruct* result);

	keys_wrapper_EXPORT keypairs_type_t *create_key_pairs_shp(struct PrivateKeyHelpInfo *privatehelper,
			struct WalletAddrHelpInfo *addrhelper);

	keys_wrapper_EXPORT char *create_asset_send_tx_shp(const char *toAddr, double quantity, const char *issueTxid, int multiple,
			const char *unspentScriptPubKey, const char *unspentTxid, uint32_t unspentVout,
			double unspentQty, const char * unspentRedeemScript, const char *privateKey,
			struct PrivateKeyHelpInfo *privatehelper, struct WalletAddrHelpInfo *addrhelper);

	keys_wrapper_EXPORT char *sign_message_shp(const char *strAddress, const char *strMessage,
			struct PrivateKeyHelpInfo *privatehelper, struct WalletAddrHelpInfo *addrhelper);

	keys_wrapper_EXPORT int verify_message_shp(const char *strAddress, const char *strSign,
			const char *strMessage, struct WalletAddrHelpInfo *addrhelper);	
#endif		

/// createStreamPublishTx 함수를 c에서 사용 하기 위해 wrapping 한 함수.
	keys_wrapper_EXPORT char* create_stream_publish_tx(const char* streamKey, const char* streamItem, const char* createTxid,
			const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
			const char* unspentRedeemScript, const char* privateKey, struct PrivateKeyHelpInfo *helper);
/// createKeyPairs 함수를 c에서 사용 하기 위해 wrapping 한 함수.
	keys_wrapper_EXPORT keypairs_type_t *create_key_pairs(struct PrivateKeyHelpInfo *privatehelper,
			struct WalletAddrHelpInfo *addrhelper);
/// createAssetSendTx 함수를 c에서 사용 하기 위해 wrapping 한 함수.
	keys_wrapper_EXPORT char *create_asset_send_tx(const char *toAddr, double quantity, const char *issueTxid, int multiple,
			const char *unspentScriptPubKey, const char *unspentTxid, uint32_t unspentVout,
			double unspentQty, const char * unspentRedeemScript, const char *privateKey,
			struct PrivateKeyHelpInfo *privatehelper, struct WalletAddrHelpInfo *addrhelper);
/// SignMessage 함수를 c에서 사용하기 위해 wrapping 한 함수.
	keys_wrapper_EXPORT char *sign_message(const char *strAddress, const char *strMessage,
			struct PrivateKeyHelpInfo *privatehelper, struct WalletAddrHelpInfo *addrhelper);
/// VerifyMessage 함수를 c에서 사용하기 위해 wrapping 한 함수.
	keys_wrapper_EXPORT int verify_message(const char *strAddress, const char *strSign,
			const char *strMessage, struct WalletAddrHelpInfo *addrhelper);

#ifdef __cplusplus
}
#endif

#endif	//HS_KEYS_WRAPPER_H
