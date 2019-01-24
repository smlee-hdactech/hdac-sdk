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

char * create_stream_publish_tx(const char* streamKey, const char* streamItem, const char* createTxid,
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
	//cout << result << ", " << result.size() << endl;
	char *retVal = static_cast<char*>(malloc(result.size() + 1));
#ifdef _WIN32	
	strcpy_s(retVal, result.size()+1, result.c_str());
#else
	strcpy(retVal, result.c_str());
#endif	
	//char *retVal = new char[100];
	//sprintf(retVal, "just test");
	return retVal;
}

#if 0
void create_stream_publish_tx1(const char* streamKey, const char* streamItem, const char* createTxid,
	const char* unspentScriptPubKey, const char* unspentTxid, uint32_t unspentVOut,
	const char* unspentRedeemScript, const char* privateKey, const char* privateKeyPrefix, const char* addrChecksum) {
	PrivateKeyHelpInfo helpInfo;
	sprintf_s(helpInfo.addrChecksum, addrChecksum);
	sprintf_s(helpInfo.privateKeyPrefix, privateKeyPrefix);

	create_stream_publish_tx(streamKey, streamItem, createTxid,
		unspentScriptPubKey, unspentTxid, unspentVOut,
		unspentRedeemScript, privateKey, &helpInfo);
}

void test_char_param(const char* streamKey) {
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
