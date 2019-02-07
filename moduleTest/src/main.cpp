#include <iostream>
#include <utils/utilstrencodings.h>
#include <structs/hs_structs.h>
#include <keys/keyshelper.h>
#include <keys/hs_keys.h>
#include <rpc/rpcresult.h>
#include <rpc/rpcclient.h>
#include <rpc/hs_rpc.h>
#include <helpers/hs_helpers.h>
#include <primitives/hs_primitives.h>

#include <json_spirit/json_spirit.h>
#include <json_spirit/json_spirit_reader_template.h>
#include <json_spirit/json_spirit_writer_template.h>

using namespace std;
using namespace json_spirit;

void testCalcSHA256()
{
    vector<unsigned char> result = obtainHash("This is test message");
    string resultStr = HexStr(result);
    std::transform(resultStr.begin(), resultStr.end(), resultStr.begin(), ::toupper);
    if (resultStr == "DDDBDC2845C9D80DC288710D9B2CF2D6C4F613D0DC4C048A9EA0E8674C2C5E73")   {
        cout << "SHA256 is correct, result: " << resultStr << endl;
    }
    cout << "SHA256 is incorrect, check the program: " << resultStr << endl;
}

// TODO : should add the logging function
// TODO : parameters from config-file.

void testCreateKeyPairs()
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    KeysHelperWithRpc helper(client);

    auto keyPairs = createKeyPairs(helper.privHelper(), helper.addrHelper());
    cout << "address : " << keyPairs.walletAddr << endl;
    cout << "pubkey ID: " << keyPairs.pubkeyHash << endl;
    cout << "pubKey: " << keyPairs.pubkey << endl;
    cout << "privkey: " << keyPairs.privateKey << endl;
}


void testPubkeyToAddrAfterGettingParams()
{
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    KeysHelperWithRpc helper(client);

    // from cli, getaddresses true
    string pubKey = "027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b";
    cout << "pubKey: " << pubKey << endl;

    //cout << hex << checksum << endl;
    string walletAddr = walletAddrFromPubKey(pubKey, helper.addrHelper());
    cout << "address : " << walletAddr << endl;
}


void testgetinfo()
{
    string resultStr;
    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234", "kcc"};
    if (!rpcResult(getinfo(client), resultStr))  {
        return;
    }
    cout << "getinfo: " << resultStr << endl;
}

class ElementGrabberToPublishStreamTx {
public:
	string _walletAddr;
	string _privateKey;
	RpcClient _client;
	string _unspentTxid;
	int32_t _unspentVoutIdx;
	string _unspentScriptPubKey;
	string _createTxid;
	KeysHelperWithRpc _helper;

private:
	void chooseUnspent() {
		string resultStr;
		if (!rpcResult(listunspent(_client, _walletAddr), resultStr)) {
			return;
		}

		Value resultValue;
		//string txid;
		//int vout;
		//string scriptPubKey;

		cout << "listunspent result: " << endl;
		if (read_string(resultStr, resultValue)) {
			if (resultValue.type() != array_type) {
				throw "wrong result format";
			}
			Array unspents = resultValue.get_array();
			Object selected;
			for (unsigned int i = 0; i < unspents.size(); i++) {
				Value value = find_value(unspents[i].get_obj(), "assets");
				if (value.type() == array_type && value.get_array().size() == 0) {
					selected = unspents[i].get_obj();
					break;
				}
			}

			_unspentTxid = find_value(selected, "txid").get_str();
			_unspentVoutIdx = find_value(selected, "vout").get_int();
			_unspentScriptPubKey = find_value(selected, "scriptPubKey").get_str();
			cout << "txid = " << _unspentTxid << endl;
			cout << "vout = " << _unspentVoutIdx << endl;
			cout << "scriptPubKey = " << _unspentScriptPubKey << endl;
		}
	}

	void chooseStream(const string& streamName) {
		string resultStr;
		if (!rpcResult(liststreams(_client, streamName), resultStr)) {
			return;
		}

		Value resultValue;
		//string createTxid;
		if (read_string(resultStr, resultValue)) {
			if (resultValue.type() != array_type) {
				throw "wrong result format";
			}
			Array unspents = resultValue.get_array();
			_createTxid = find_value(unspents[0].get_obj(), "createtxid").get_str();
			cout << "createtxid = " << _createTxid << endl;
		}
	}

public:
	ElementGrabberToPublishStreamTx(const string &streamName) :
		_client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" },
		_helper{ _client }	{
		vector<tuple<string, string>> addrNPrivates = {
			make_tuple("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5", "V6X4NaaDQSTgXdAcCzUrSxWqAuFcd53TRXRqmSafUYEbY5DgGMitPEzk"),
			make_tuple("18wD7MBodeTYRAvN5bRuWYB11jwHdkGVCBLSnB", "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp"),
			make_tuple("1EpVCtEHe61hVdgQLKSzM8ZyeFJGdsey29sMQi", "V9ugoEazm16SKbvj7DVxMUcXQnvKpBPaeZ3KEUxTUWoChXQTuHKyzbKx")
		};

		int selected = 1;
		tie(_walletAddr, _privateKey) = addrNPrivates[selected];

		//RpcClient client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" };
		chooseUnspent();

		string resultStr;
		if (!rpcResult(lockunspent(_client, false, _unspentTxid, _unspentVoutIdx), resultStr)) {
			throw "cannot call lockunspent";
		}

		if (!rpcResult(lockunspent(_client, true, _unspentTxid, _unspentVoutIdx), resultStr)) {
			throw "cannot call listlockunspent";
		}

		chooseStream(streamName);
	}
};

void testRawTransactionForStreamPublish()
{
	ElementGrabberToPublishStreamTx grabber("stream9");
	string txHex = createStreamPublishTx("key1", "first programmed version", grabber._createTxid,
		grabber._unspentScriptPubKey, grabber._unspentTxid, grabber._unspentVoutIdx, "", 
		grabber._privateKey, grabber._helper.privHelper());

    cout << "raw transaction: " << txHex << endl;
}

class ElementGrabberToSendAssetTx {
public:
	string _walletAddr;
	string _privateKey;
	RpcClient _client;
	string _unspentTxid;
	int32_t _unspentVoutIdx;
	string _unspentScriptPubKey;
	string _unspentRedeemScript;
	string _issueTxid;
	int _multiple;
	double _unspentQty;
	KeysHelperWithRpc _helper;

private:
	void chooseUnspent(const string& assetName) {
		string resultStr;
		if (!rpcResult(listunspent(_client, _walletAddr), resultStr)) {
			throw "cannot call listunspent";
		}

		Value resultValue;
		string txid;
		//int vout;
		string scriptPubKey;
		//double unspentQty;
		
		cout << "listunspent result: " << endl;
		if (read_string(resultStr, resultValue)) {
			if (resultValue.type() != array_type) {
				throw "wrong result format";
			}

			Array unspents = resultValue.get_array();
			Object selected;
			for (unsigned int i = 0; i < unspents.size(); i++) {
				Value value = find_value(unspents[i].get_obj(), "assets");
				if (value.type() == array_type && value.get_array().size() != 0) {
					bool found = false;
					Array assetInfo = value.get_array();
					for (unsigned int i = 0; i < assetInfo.size(); i++) {
						//for (const Object& assetInfo : value.get_array())   {
						if (assetInfo[i].type() == obj_type) {
							Value name = find_value(assetInfo[i].get_obj(), "name");
							if (name.get_str() == assetName) {
								found = true;
								Value qty = find_value(assetInfo[i].get_obj(), "qty");
								_unspentQty = qty.get_real();
								break;
							}
						}
					}
					if (found == true) {
						selected = unspents[i].get_obj();
						break;
					}
				}
			}

			_unspentTxid = find_value(selected, "txid").get_str();
			_unspentVoutIdx = find_value(selected, "vout").get_int();
			_unspentScriptPubKey = find_value(selected, "scriptPubKey").get_str();
			_unspentRedeemScript = find_value(selected, "redeemScript").get_str();
			cout << "txid = " << _unspentTxid << endl;
			cout << "vout = " << _unspentVoutIdx << endl;
			cout << "scriptPubKey = " << _unspentScriptPubKey << endl;
			cout << "redeemScript = " << _unspentRedeemScript << endl;
		}
	}

	void chooseAsset(const string& assetName) {
		string resultStr;
		if (!rpcResult(listassets(_client, assetName), resultStr)) {
			throw "cannot call listassets";
		}

		Value resultValue;

		if (read_string(resultStr, resultValue)) {
			if (resultValue.type() != array_type) {
				throw "wrong result format";
			}
			Array assets = resultValue.get_array();
			_issueTxid = find_value(assets[0].get_obj(), "issuetxid").get_str();
			cout << "issuetxid = " << _issueTxid << endl;
			_multiple = find_value(assets[0].get_obj(), "multiple").get_int();
			cout << "multiple = " << _multiple << endl;
		}
	}

public:
	
	ElementGrabberToSendAssetTx(const string &assetName) :
		ElementGrabberToSendAssetTx(assetName, "18wD7MBodeTYRAvN5bRuWYB11jwHdkGVCBLSnB", "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp")
	{
	}
	ElementGrabberToSendAssetTx(const string &assetName, const string &walletAddr, const string &privateKey) :
		_client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" },
		_helper{ _client }	{
#if 0
		vector<tuple<string, string>> addrNPrivates = {
			make_tuple("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5", "V6X4NaaDQSTgXdAcCzUrSxWqAuFcd53TRXRqmSafUYEbY5DgGMitPEzk"),
			make_tuple("18wD7MBodeTYRAvN5bRuWYB11jwHdkGVCBLSnB", "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp"),
			make_tuple("1EpVCtEHe61hVdgQLKSzM8ZyeFJGdsey29sMQi", "V9ugoEazm16SKbvj7DVxMUcXQnvKpBPaeZ3KEUxTUWoChXQTuHKyzbKx")
		};

		int selected = 1;
		tie(_walletAddr, _privateKey) = addrNPrivates[selected];
#else
		_walletAddr = walletAddr;
		_privateKey = privateKey;
#endif

		//RpcClient client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" };
		chooseUnspent(assetName);

		string resultStr;
		if (!rpcResult(lockunspent(_client, false, _unspentTxid, _unspentVoutIdx), resultStr)) {
			throw "cannot call lockunspent";
		}

		if (!rpcResult(lockunspent(_client, true, _unspentTxid, _unspentVoutIdx), resultStr)) {
			throw "cannot call listlockunspent";
		}

		chooseAsset(assetName);
	}
};

string testRawTransactionForAssetSend()
{
	ElementGrabberToSendAssetTx grabber("ass1");

	string txHex = createAssetSendTx("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5",
		10, grabber._issueTxid, grabber._multiple,
		grabber._unspentScriptPubKey, grabber._unspentTxid,
		grabber._unspentVoutIdx,
		grabber._unspentQty, "", grabber._privateKey,
		grabber._helper.privHelper(),
		grabber._helper.addrHelper());

    cout << "raw transaction: " << txHex << endl;
	return txHex;
}

string testRawTransactionForAssetSendFromMultiSigAddr()
{
	ElementGrabberToSendAssetTx grabber("testAsset2", "4VUofBRAsTtF4KJ9gD9TMx3Ek1Eyr1WozSrDMU", "V6X4NaaDQSTgXdAcCzUrSxWqAuFcd53TRXRqmSafUYEbY5DgGMitPEzk");

	string txHex = createAssetSendTx("12HL7VoKwqGV8ufRMMXWC5gcJ1MJEWCYW5KnTN",
		10, grabber._issueTxid, grabber._multiple,
		grabber._unspentScriptPubKey, grabber._unspentTxid,
		grabber._unspentVoutIdx,
		grabber._unspentQty, grabber._unspentRedeemScript, grabber._privateKey,
		grabber._helper.privHelper(),
		grabber._helper.addrHelper());

	cout << "raw transaction: " << txHex << endl;
	return txHex;

}

string testRawTransactionForAssetSendToMultiSigAddr()
{
	ElementGrabberToSendAssetTx grabber("testAsset2");	

	string txHex = createAssetSendTx("49X6rkfZYidXVTPNRDiZHhnxFG7RmMLrQxF7yr",
		10, grabber._issueTxid, grabber._multiple,
		grabber._unspentScriptPubKey, grabber._unspentTxid,
		grabber._unspentVoutIdx,
		grabber._unspentQty, "", grabber._privateKey,
		grabber._helper.privHelper(),
		grabber._helper.addrHelper());

	cout << "raw transaction: " << txHex << endl;
	return txHex;

}

void testHashFromFile()
{
    auto result = hashFromFile("video1.mp4");
    cout << "file hash: " << HexStr(result) << endl;
}


std::function<string(const std::vector<unsigned char>& vch, bool isScriptHash)> convertAddrDefault(const KeysHelperWithRpc& helper)
{
	auto convertAddr = [&helper](const std::vector<unsigned char>& vch, bool isScriptHash) -> string {
		ostringstream oStm;
		if (isScriptHash) {
			CBitcoinAddress addrH(CScriptID(uint160(vch)), helper.addrHelper());
			if (addrH.IsValid()) {
				//cout << addrH.ToString() << endl;
				oStm << addrH.ToString();
			}
		}
		else {
			CBitcoinAddress addr(CKeyID(uint160(vch)), helper.addrHelper());
			if (addr.IsValid()) {
				//cout << addr.ToString() << endl;
				oStm << addr.ToString();
			}
		}
		return oStm.str();
	};
	return convertAddr;
}

void testAnalyzeTx(const string& txStr)
{
	RpcClient client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" };
	KeysHelperWithRpc helper(client);

	auto convertAddr = convertAddrDefault(helper);

	auto jsonResult = analyzeTx(txStr, convertAddr);
	auto result = write_string(Value(jsonResult), true);
	cout << result << endl;
}

void testAnalyzeMultisig()
{
	RpcClient client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" };
	KeysHelperWithRpc helper(client);

	string testStr = "0100000001eeaf0c4c355f14b84266a1f5fa6341f8e93b0e48917b8813eb76928a39c6bc2e000000006b483045022100bd876d60cca7c02007f2852ba6dcaad78bfa21a6f46cde9bd1826e3ec6381b420220241c8f7b1ae8bb6147c8d7df5b9aab0f47a257c2f5ad44309aa410dc19cf050201210287b7aeb453d64da31c20c316a7ec11b484327b3881b13d5f3818ff34ff821d6effffffff02000000000000000035a9144671c47a9d20c240a291661520d4af51df08fb0b871c73706b712f2a1e55743c2cf767b55a1cf30af3a90a000000000000007500000000000000001976a914d806940ddc6d27dab4c2de6f1d6aca0aff20d0c688ac00000000";
	auto convertAddr = convertAddrDefault(helper);

	Object jsonResult = analyzeTx(testStr, convertAddr);

	auto result = write_string(Value(jsonResult), true);
	cout << "multisig result: " << endl;
	cout << result << endl;

}

void testAnalyzeFromMultisig()
{
	RpcClient client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" };
	KeysHelperWithRpc helper(client);

	//string testStr = "0100000001ec0973036513d26b1ae2ec3525ed8a3ed37a98689367cf70bdfa652300ce46b0000000009100473044022034d74b5d0b69de446ffbde802ae48772b8c346040fcaa8260ef659dfc789011302202c40bf851dc7efdfc4b8b50679bb32d5bd54e3f7e9d0769d1bfb79c2218a79f201475221027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b21038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd52aeffffffff0200000000000000003776a9143ab53060d41b5fa662a2d4575a69464b5759839588ac1c73706b712f2a1e55743c2cf767b55a1cf30af3a90a0000000000000075000000000000000035a9143e45d3a48882576ad5900978303705e1a6000305871c73706b712f2a1e55743c2cf767b55a1cf30af3a95a000000000000007500000000";
	string testStr = "0100000001e448d487e53b4fbe6215b0536ae64a7a034211f4811a93c0a5418ff5dc3d00760000000091004730440220570b5aef89263eeb73839d1f505d0e78cb28b81defb0ee72e86a6da8ca760ca80220301dbb484ac6d7f8c77fef516ece4f833f4c44bb46405e2617117c0bccdd8dbc014752210287b7aeb453d64da31c20c316a7ec11b484327b3881b13d5f3818ff34ff821d6e2103a3864876b0424f56c0f38d3f8f948fe3477c53e2ee1fb5da01f58466d0a9642d52aeffffffff0200000000000000003776a914097b6744e025e751e6cbb5fef1f557e1ca590ed588ac1c73706b712f2a1e55743c2cf767b55a1cf30af3a90a0000000000000075000000000000000035a914da2144b8eb31ffae3de2b2ea0a5da85d575dcdbc871c73706b712f2a1e55743c2cf767b55a1cf30af3a95a000000000000007500000000";
	auto convertAddr = convertAddrDefault(helper);

	Object jsonResult = analyzeTx(testStr, convertAddr);

	auto result = write_string(Value(jsonResult), true);
	cout << "from multisig result: " << endl;
	cout << result << endl;
	
}

void testAnalyzeFromMultisigCompleted()
{
	RpcClient client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" };
	KeysHelperWithRpc helper(client);

	string testStr = "0100000001e448d487e53b4fbe6215b0536ae64a7a034211f4811a93c0a5418ff5dc3d007600000000d9004730440220570b5aef89263eeb73839d1f505d0e78cb28b81defb0ee72e86a6da8ca760ca80220301dbb484ac6d7f8c77fef516ece4f833f4c44bb46405e2617117c0bccdd8dbc01473044022061c159e278057db2cac8ab94a548e3d5d588b085c77be9ec5b9c53462199422b02207f96f14dbb1b6989ffb0ffa8825125c4eef829f00823c114da41a61e5bc334a1014752210287b7aeb453d64da31c20c316a7ec11b484327b3881b13d5f3818ff34ff821d6e2103a3864876b0424f56c0f38d3f8f948fe3477c53e2ee1fb5da01f58466d0a9642d52aeffffffff0200000000000000003776a914097b6744e025e751e6cbb5fef1f557e1ca590ed588ac1c73706b712f2a1e55743c2cf767b55a1cf30af3a90a0000000000000075000000000000000035a914da2144b8eb31ffae3de2b2ea0a5da85d575dcdbc871c73706b712f2a1e55743c2cf767b55a1cf30af3a95a000000000000007500000000";
	auto convertAddr = convertAddrDefault(helper);

	Object jsonResult = analyzeTx(testStr, convertAddr);

	auto result = write_string(Value(jsonResult), true);
	cout << "from multisig complete result: " << endl;
	cout << result << endl;

}

void checkMultisigAddr()
{
	RpcClient client{ "13.125.145.98", 4260, "hdacrpc", "1234", "kcc" };
	KeysHelperWithRpc helper(client);

	//CPubKey firstPubKey(ParseHex("027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b"));
	//CPubKey secondPubKey(ParseHex("038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd"));
	CPubKey firstPubKey(ParseHex("021a52ff7f6fc6e10a5c52a4c4a4c4b4c3611387a19a57d40e36b2adce55263ebf"));
	CPubKey secondPubKey(ParseHex("0355ae7eab8713fd9f8cddcb18f67562e51c7bbb79c2fe3f9531403442499643f5"));


	CBitcoinAddress firstAddr(firstPubKey.GetID(), helper.addrHelper());
	CBitcoinAddress secondAddr(secondPubKey.GetID(), helper.addrHelper());
	
	cout << "first addr: " << firstAddr.ToString() << endl;
	cout << "second addr: " << secondAddr.ToString() << endl;
}

int main()
{
#if 0
    cout << "1. test sha256 of \"This is test message\"" << endl;
    testCalcSHA256();

    cout << "2. test sha256 of file, video1.mp4" << endl;
    testHashFromFile();

    cout << "3. test getinfo" << endl;
    testgetinfo();

    cout << "4. create key pairs" << endl;
    testCreateKeyPairs();

    cout << "5. pubkey test" << endl;
    testPubkeyToAddrAfterGettingParams();

    cout << "6. stream publish test" << endl;
    try {
        testRawTransactionForStreamPublish();
    } catch(std::exception &e) {
        cout << e.what() << endl;
    }

    cout << "7. asset send test" << endl;
	string assetSendTx;
    try {
		assetSendTx = testRawTransactionForAssetSend();
    } catch(std::exception &e) {
        cout << e.what() << endl;
    }
	   
	cout << "8. test analyze" << endl;
	testAnalyzeTx(assetSendTx);

	cout << "9. multisig anlaysis" << endl;
	testAnalyzeMultisig();
#endif
	cout << "10. asset send to multisig addr test" << endl;
	//string assetSendTx;
	string assetSendTx1;
	try {
		assetSendTx1 = testRawTransactionForAssetSendToMultiSigAddr();
	}
	catch (std::exception &e) {
		cout << e.what() << endl;
	}

	cout << "11. test analyze for No. 10" << endl;
	testAnalyzeTx(assetSendTx1);


	cout << "12. multisig anlaysis" << endl;
	testAnalyzeFromMultisig();

#if 0
	cout << "13. check multisig addr" << endl;
	checkMultisigAddr();
#endif

	cout << "14. completed multisig anlaysis" << endl;
	testAnalyzeFromMultisigCompleted();

	cout << "15. from multisig tx" << endl;
	string assetSendTx2;
	try {
		assetSendTx2 = testRawTransactionForAssetSendFromMultiSigAddr();
	}
	catch (std::exception &e) {
		cout << e.what() << endl;
	}
	cout << "16. test analyze for No. 15" << endl;
	testAnalyzeTx(assetSendTx2);

    return 0;
}
