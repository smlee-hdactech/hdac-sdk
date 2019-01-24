#include <iostream>
#include <rpc/rpcresult.h>
#include <rpc/rpcclient.h>
#include <rpc/hs_rpc.h>
#include <keys/hs_keys.h>
#include <helpers/hs_helpers.h>

#include <json_spirit/json_spirit_reader_template.h>
#include <json_spirit/json_spirit_writer_template.h>

using namespace std;
using namespace json_spirit;

void testgetinfo(void)
{
	string resultStr;
	RpcClient client{"192.168.70.162", 2684, "hdacrpc", "1234", "mykcc"};
	if (!rpcResult(getinfo(client), resultStr))  {
		return;
	}
	cout << "getinfo: " << resultStr << endl;
}

void testLocalSigntoLocalVerify(void)
{
	// params set
	KeysHelperWithFileAll helper("params.dat");

	// key pairs set
	auto keyPairs = createKeyPairs(helper.privHelper(), helper.addrHelper());
	cout << "address : " << keyPairs.walletAddr << endl;
	cout << "pubkey ID: " << keyPairs.pubkeyHash << endl;
	cout << "pubKey: " << keyPairs.pubkey << endl;
	cout << "privkey: " << keyPairs.privateKey << endl;

	// text set
	string text = "Hdac Technology, Solution Dev Team, Test Text.";
	cout << "text [" << text << "]" << endl;

	// sign message
	string signMsg = SignMessage(
			keyPairs.privateKey,
			text,
			helper.privHelper(),
			helper.addrHelper());

	// print signed message
	cout << "sign message = [" << signMsg << "]" << endl; 

	// verify message
	bool res = VerifyMessage(
			keyPairs.walletAddr,
			signMsg,
			text,
			helper.addrHelper());

	// print verify result
	if (res) {
		cout << "verify message result = ["<< "true" << "]" << endl;
	} else {
		cout << "verify message result = ["<< "false" << "]" << endl;
	}
}

void testLocalSigntoRpcVerify(void)
{
	// params set
	KeysHelperWithFileAll helper("params.dat");

	// key pairs set
	auto keyPairs = createKeyPairs(helper.privHelper(), helper.addrHelper());
	cout << "address : " << keyPairs.walletAddr << endl;
	cout << "pubkey ID: " << keyPairs.pubkeyHash << endl;
	cout << "pubKey: " << keyPairs.pubkey << endl;
	cout << "privkey: " << keyPairs.privateKey << endl;

	// text set
	string text = "Hdac Technology, Solution Dev Team, Test Text.";
	cout << "text [" << text << "]" << endl;

	// sign message
	string signMsg = SignMessage(
			keyPairs.privateKey,
			text,
			helper.privHelper(),
			helper.addrHelper());

	// print signed message
	cout << "sign message = [" << signMsg << "]" << endl; 

	// rpc to verify message
	string resultStr;
	RpcClient client{"192.168.70.162", 2684, "hdacrpc", "1234", "mykcc"};
	if (!rpcResult(verifymessage(client,
			keyPairs.walletAddr, signMsg, text), resultStr))  {
		return;
	}
	cout << "verifymessage: " << resultStr << endl;
}

void testRpcSigntoLocalVerify(void)
{
	// params set
	KeysHelperWithFileAll helper("params.dat");

	// key pairs set
	auto keyPairs = createKeyPairs(helper.privHelper(), helper.addrHelper());
	cout << "address : " << keyPairs.walletAddr << endl;
	cout << "pubkey ID: " << keyPairs.pubkeyHash << endl;
	cout << "pubKey: " << keyPairs.pubkey << endl;
	cout << "privkey: " << keyPairs.privateKey << endl;

	// text set
	string text = "Hdac Technology, Solution Dev Team, Test Text.";
	cout << "text [" << text << "]" << endl;
	
	// rpc to sign message
	string signMsg;
	RpcClient client{"192.168.70.162", 2684, "hdacrpc", "1234", "mykcc"};
	if (!rpcResult(signmessage(client,
			keyPairs.privateKey, text), signMsg))  {
		return;
	}

	// print signed message
	cout << "sign message = [" << signMsg << "]" << endl; 

	// verify message
	bool res = VerifyMessage(
			keyPairs.walletAddr,
			signMsg,
			text,
			helper.addrHelper());

	// print verify result
	if (res) {
		cout << "verify message result = ["<< "true" << "]" << endl;
	} else {
		cout << "verify message result = ["<< "false" << "]" << endl;
	}
}

void testRpcSigntoRpcVerify(void)
{
	// params set
	KeysHelperWithFileAll helper("params.dat");

	// key pairs set
	auto keyPairs = createKeyPairs(helper.privHelper(), helper.addrHelper());
	cout << "address : " << keyPairs.walletAddr << endl;
	cout << "pubkey ID: " << keyPairs.pubkeyHash << endl;
	cout << "pubKey: " << keyPairs.pubkey << endl;
	cout << "privkey: " << keyPairs.privateKey << endl;
	
	// text set
	string text = "Hdac Technology, Solution Dev Team, Test Text.";
	cout << "text [" << text << "]" << endl;

	// rpc set	
	RpcClient client{"192.168.70.162", 2684, "hdacrpc", "1234", "mykcc"};

	// rpc to sign message
	string signMsg;
	if (!rpcResult(signmessage(client,
			keyPairs.privateKey, text), signMsg))  {
		return;
	}

	// print signed message
	cout << "sign message = [" << signMsg << "]" << endl; 

	// rpc to verify message
	string resultStr;
	if (!rpcResult(verifymessage(client,
			keyPairs.walletAddr, signMsg, text), resultStr))  {
		return;
	}
	cout << "verifymessage: " << resultStr << endl;
}

int main(void)
{
	// getinfo test, node connect check
	cout << "1. geinfo test" << endl;
	testgetinfo();

	// local sign, local verify case
	cout << "2. test local sign, Local verify message" << endl;
	testLocalSigntoLocalVerify();
	
	// local sign, rpc verify case
	cout << "3. test Local sign, RPC verify message" << endl;
	testLocalSigntoRpcVerify();
	
	// rpc sign, local verify case
	cout << "4. test RPC sign, Local verify message" << endl;
	testRpcSigntoLocalVerify();

	// rpc sign, rpc verify case
	cout << "5. test RPC sign, RPC verify message" << endl;
	testRpcSigntoRpcVerify();

	return 0;
}

