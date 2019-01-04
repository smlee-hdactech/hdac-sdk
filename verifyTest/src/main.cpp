#include <boost/scoped_ptr.hpp>
#include <keys/key.h>
#include <keys/bitcoinaddress.h>
#include <utils/utilsfront.h>
#include <utils/base64.h>
#include <helpers/hs_helpers.h>

using namespace std;

const string strMessageMagic = "Hdac Signed Message:\n";

static boost::scoped_ptr<ECCVerifyHandle> globalVerifyHandle;

bool my_verifymessage(string strAddress, string strSign, string strMessage)
{
	KeysHelperWithFileAll helper("/home/aiadmin/.hdac/mykcc/params.dat");

	CBitcoinAddress addr(strAddress, helper.addrHelper());
	if (!addr.IsValid()) {
		cout << "addr error" << endl;
		return false;
	}

	CKeyID keyID;
	if (!addr.GetKeyID(keyID)) {
		cout << "get key id error" << endl;
		return false;
	}

	bool fInvalid = false;
	vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

	if (fInvalid) {
		cout << "decode base 64 error" << endl;
		return false;
	}

	CHashWriter ss(SER_GETHASH, 0);
	ss << strMessageMagic;
	ss << strMessage;

	CPubKey pubkey;
	if (!pubkey.RecoverCompact(ss.GetHash(), vchSig)) {
		return false;
	}

	return (pubkey.GetID() == keyID);
}

//int main(int argc, char *argv[])
int main(void)
{
	// Initialize elliptic curve code
	ECC_Start();
	globalVerifyHandle.reset(new ECCVerifyHandle());

	bool res = my_verifymessage(
			"1R2SGUnMeaWd59Gp9HLd2W3ADpFur1Wfz6wNYw",
			"IKqVHd0J/e6pkxbxinrZAIlIeajXncwl/wjKdQ0K4k0NZ7kuz29wpI/OJvEmlixmTWt1lw9sgBWRGP+87piYJXc=",
			"abc");

	if (res) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	return 0;
}

