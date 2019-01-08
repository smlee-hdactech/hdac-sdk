#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <keys/key.h>
#include <keys/hs_keys.h>
#include <helpers/hs_helpers.h>

using namespace std;

static boost::scoped_ptr<ECCVerifyHandle> globalVerifyHandle;

int main(void)
{
	// strings = address, publickey, privatekey, text
        string address = "1X2LwU1De6stwo23DNHWTPY5txdVDUiJPopK9K";
        string pubkey = "0380652428b950fb13c2d3710b3f320ce07cf95d46cda54a8b1a589051471d7050";
        string privkey = "V7R1AtfQke1QX9rLWRhF3wQj5xJZNV5DZzcZSTUbHz2gkFnZcPEhmAaF";
	string text = "Hdac Technology, Solution DEV Team, Test Text.";

	// Initialize elliptic curve code
	ECC_Start();
	globalVerifyHandle.reset(new ECCVerifyHandle());


	// print
	cout << "address = " << address << endl;
	cout << "private key = " << privkey << endl;
	cout << "text = " << text << endl;

	// params set
	KeysHelperWithFileAll helper("params.dat");

	// sign message
	string signMsg = signmessage(
			privkey,
			text,
			helper.privHelper(),
			helper.addrHelper());

	// print signed message
	cout << "sign message = [" << signMsg << "]" << endl; 

	// verify message
	bool res = verifymessage(
			address,
			signMsg,
			text,
			helper.addrHelper());

	// print verify result
	if (res) {
		cout << "verify message result = ["<< "true" << "]" << endl;
	} else {
		cout << "verify message result = ["<< "false" << "]" << endl;
	}

	return 0;
}

