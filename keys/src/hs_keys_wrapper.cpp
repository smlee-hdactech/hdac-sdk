#include <iostream>

#define CHECK_EXPORT
#ifdef CHECK_EXPORT
#include <cstdint>
#include <string>
#include <vector>
#include "hs_keys.h"
#include "keyshelper.h"
#include <utils/utilstrencodings.h>
#endif

using namespace std;

#ifdef CHECK_EXPORT 
struct PrivateKeyHelpInfo {
	char privateKeyPrefix[10];
	char addrChecksum[10];
};
#endif

extern "C" {
	__declspec(dllexport) void hello() {
		cout << "hello world" << endl;
	}
#ifdef CHECK_EXPORT
	__declspec(dllexport) void create_stream_publish_tx(const char* streamKey, const char* streamItem, const char* createTxid,
		const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
		const char* unspentRedeemScript, const char* privateKey, struct PrivateKeyHelpInfo *helper) {

		struct PrivateKeyHelperConstant : public IPrivateKeyHelper {
			PrivateKeyHelperConstant(const char* privateKeyPrefix, const char* addrChecksum) :
				_prefix(ParseHex(privateKeyPrefix)) {
				_checksum = parseHexToInt32Le(addrChecksum);
			}

			const std::vector<unsigned char> privkeyPrefix() const {
				return _prefix;
			}

			int32_t addrChecksumValue() const {
				return _checksum;
			}

			vector<unsigned char> _prefix;
			int32_t _checksum;
		};
		PrivateKeyHelperConstant privHelper(helper->privateKeyPrefix, helper->addrChecksum);

		string result = createStreamPublishTx(streamKey, streamItem, createTxid, unspentScriptPubKey, unspentTxid,
			unspentVOut, unspentRedeemScript, privateKey, privHelper);
		cout << result << endl;

	}

	__declspec(dllexport) void create_stream_publish_tx1(const char* streamKey, const char* streamItem, const char* createTxid,
		const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
		const char* unspentRedeemScript, const char* privateKey, const char* privateKeyPrefix, const char* addrChecksum) {
		PrivateKeyHelpInfo helpInfo;
		sprintf_s(helpInfo.addrChecksum, addrChecksum);
		sprintf_s(helpInfo.privateKeyPrefix, privateKeyPrefix);

		create_stream_publish_tx(streamKey, streamItem, createTxid,
			unspentScriptPubKey, unspentTxid, unspentVOut,
			unspentRedeemScript, privateKey, &helpInfo);
	}

	__declspec(dllexport) void test_char_param(const char* streamKey) {
		PrivateKeyHelpInfo helpInfo;
		sprintf_s(helpInfo.addrChecksum, "cb507245");
		sprintf_s(helpInfo.privateKeyPrefix, "8075fa23");

		struct PrivateKeyHelperConstant : public IPrivateKeyHelper {
			PrivateKeyHelperConstant(const char* privateKeyPrefix, const char* addrChecksum) :
				_prefix(ParseHex(privateKeyPrefix)) {
				_checksum = parseHexToInt32Le(addrChecksum);
			}

			const std::vector<unsigned char> privkeyPrefix() const {
				return _prefix;
			}

			int32_t addrChecksumValue() const {
				return _checksum;
			}

			vector<unsigned char> _prefix;
			int32_t _checksum;
		};
		PrivateKeyHelperConstant privHelper(helpInfo.privateKeyPrefix, helpInfo.addrChecksum);

		string result = createStreamPublishTx(streamKey, "tested by moony",
			"a0b59e8c6f2fd144485d19632f62708f88116fb11a46411dd7d1e211ec92ce9a",
			"76a9143ab53060d41b5fa662a2d4575a69464b5759839588ac1473706b700700000000000000ffffffff319ffb5b75",
			"88a98467f24a3935156496283c1d06b2fe61b86b0d6276d14ad4ef6bcb25ffd5", 0,
			"",
			"VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp",
			privHelper);
		cout << result << endl;
	}
#endif
}