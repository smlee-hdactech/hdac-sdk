#include <stdint.h>
#include <keys/hs_keys_wrapper.h>
#include <stdio.h>
#include <stdlib.h>

void test_create_stream_publish_tx(void)
{
	struct PrivateKeyHelpInfo privinfo = {
		"8075fa23", "cb507245"
	};

	char * rawTx = create_stream_publish_tx("key1", "tested by moony",
			"a0b59e8c6f2fd144485d19632f62708f88116fb11a46411dd7d1e211ec92ce9a",
			"a9143e45d3a48882576ad5900978303705e1a6000305871473706b6511feed9499be6fb101e0f59119d3fe15751473706b700800000000000000fffffffffbfe095c75",
			"db84077722b74c9c9a799cf58d6c7a265f214f003b5ef15cae368a8add8d33f8", 0,
			"5221027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b21038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd52ae",
			"VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp", &privinfo);

	printf("raw-Tx: %s\n", rawTx);
	free(rawTx);
}

void test_create_key_pairs(void)
{
	struct PrivateKeyHelpInfo privinfo = {
		"8075fa23", "cb507245"
	};

	struct WalletAddrHelpInfo addrinfo = {
		"003fd61c", "0571a3e6", "cb507245"
	};

	keypairs_type_t *keypairs = create_key_pairs(&privinfo, &addrinfo);

	printf("address : %s\n", keypairs->walletaddr);
	printf("pubkeyhash : %s\n", keypairs->pubkeyhash);
	printf("pubkey : %s\n", keypairs->pubkey);
	printf("privatekey : %s\n", keypairs->privatekey);
	free(keypairs);
}

void test_create_asset_send_tx(void)
{
	struct PrivateKeyHelpInfo privinfo = {
		"8075fa23", "cb507245"
	};

	struct WalletAddrHelpInfo addrinfo = {
		"003fd61c", "0571a3e6", "cb507245"
	};

	char * rawassetTx = create_asset_send_tx("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5", 10,
			"44fdb8103f4e13d6ef2011d54933f2747b455c613b3cfe4886d187330d50b640", 10,
			"76a9143ab53060d41b5fa662a2d4575a69464b5759839588ac1c73706b7174f23349d51120efd6134e3f10b8fd44ac2600000000000075",
			"030374d736a70c5faf5d16887d2263e812cb896938bedeefd44c128417e2460a", 1,
			990.0,
			"",
			"VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp",
			&privinfo, &addrinfo);

	printf("raw-asset-Tx: %s\n", rawassetTx);
	free(rawassetTx);
}

void test_sign_message(void)
{
	struct PrivateKeyHelpInfo privinfo = {
		"8075fa23", "cb507245"
	};

	struct WalletAddrHelpInfo addrinfo = {
		"003fd61c", "0571a3e6", "cb507245"
	};

	char * signMessage = sign_message(
			"VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp",
			"Hdac Technology, Solution Dev Team, Test Text.",
			&privinfo, &addrinfo);

	printf("sign-Message: %s\n", signMessage);
	free(signMessage);
}

void test_verify_message(void)
{
	struct WalletAddrHelpInfo addrinfo = {
		"003fd61c", "0571a3e6", "cb507245"
	};

	int verify_check = verify_message(
			"18wD7MBodeTYRAvN5bRuWYB11jwHdkGVCBLSnB",
			"IJKPyPUFEgnlrcixdqbfgAks89Gi29uzGAyMUYICz8VAWEs6VlOpjzregZ2WrcarZoNtXD7aLC2S6VWJ8XowH9c=",
			"Hdac Technology, Solution Dev Team, Test Text.",
			&addrinfo);

	printf("verify-Message: %s\n", verify_check? "true" : "false" );
}

int main()
{
	test_create_stream_publish_tx();

	test_create_key_pairs();

	test_create_asset_send_tx();
	
	test_sign_message();

	test_verify_message();

	return 0;
}