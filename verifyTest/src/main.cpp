#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <keys/key.h>
#include <keys/hs_keys.h>
#include <helpers/hs_helpers.h>

using namespace std;

static boost::scoped_ptr<ECCVerifyHandle> globalVerifyHandle;

//int main(int argc, char *argv[])
int main(void)
{
	// Initialize elliptic curve code
	ECC_Start();
	globalVerifyHandle.reset(new ECCVerifyHandle());

	KeysHelperWithFileAll helper("params.dat");

	bool res = verifymessage(
			"1R2SGUnMeaWd59Gp9HLd2W3ADpFur1Wfz6wNYw",
			"IKqVHd0J/e6pkxbxinrZAIlIeajXncwl/wjKdQ0K4k0NZ7kuz29wpI/OJvEmlixmTWt1lw9sgBWRGP+87piYJXc=",
			"abc",
			helper.addrHelper());

	if (res) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	return 0;
}

