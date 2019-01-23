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

struct PrivateKeyHelpInfo {
	char privateKeyPrefix[10];
	char addrChecksum[10];
};

#ifdef __cplusplus
extern "C" {
#endif

	keys_wrapper_EXPORT char* create_stream_publish_tx_shp(const char* streamKey, const char* streamItem, const char* createTxid,
		const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
		const char* unspentRedeemScript, const char* privateKey, struct PrivateKeyHelpInfo *helper);
	keys_wrapper_EXPORT char* create_stream_publish_tx(const char* streamKey, const char* streamItem, const char* createTxid,
		const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
		const char* unspentRedeemScript, const char* privateKey, struct PrivateKeyHelpInfo *helper);
#if 0
	keys_wrapper_EXPORT void create_stream_publish_tx1(const char* streamKey, const char* streamItem, const char* createTxid,
		const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
		const char* unspentRedeemScript, const char* privateKey, const char* privateKeyPrefix, const char* addrChecksum);
	keys_wrapper_EXPORT void test_char_param(const char* streamKey);
#endif

#ifdef __cplusplus
}
#endif

#endif	//HS_KEYS_WRAPPER_H