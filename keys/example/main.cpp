#include <keys/bitcoinaddress.h>
#include <iostream>
#include <utils/utilsfront.h>
#include <keys/multisig.h>
#include <keys/transactions.h>

using namespace std;

class SampleWalletAddrHelper : public IWalletAddrHelper {
public:
    SampleWalletAddrHelper() { }

    const std::vector<unsigned char> pubkeyAddrPrefix() const override  {
        return ParseHex("003fd61c");
    }
    const std::vector<unsigned char> scriptAddrPrefix() const override  {
        return ParseHex("0571a3e6");
    }

    int32_t addrChecksumValue() const override {
        return parseHexToInt32Le("cb507245");
    }
};

class SamplePrivateKeyHelper : public IPrivateKeyHelper {
public:
    SamplePrivateKeyHelper() { }

    const std::vector<unsigned char> privkeyPrefix() const override {
        return ParseHex("8075fa23");
    }

    int32_t addrChecksumValue() const override {
        return parseHexToInt32Le("cb507245");
    }
};

void testPubkeyToAddr()
{
    CPubKey pubkey(ParseHex("027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b"));
    cout << "pubKey: " << HexStr(pubkey) << endl;

    //cout << hex << checksum << endl;
    CBitcoinAddress addr(pubkey.GetID(), SampleWalletAddrHelper());

    cout << "address : " << addr.ToString() << endl;
}

void testMultisigScript()
{
    // pubkeys from "getaddresses true", pubkey
    vector<string> pubkeys = {
        "027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b",
        "038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd"
    };

    MultisigAddrInfo info;
    createMultisigInfo(pubkeys, 2, SampleWalletAddrHelper(), info);

    cout << "multisig-addr: " << info.addr << endl;
    cout << "redeemScript: " << info.redeemScript << endl;
}

void testCreateRawSendFrom()
{
    // streamKey : 1st param, user
    // streamItem : 2nd param, user
    // createTxid : 3rd param, from "liststreams ${streamName}"
    // unspentScriptPubKey : 4th param, from listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', scriptPubKey
    // unspentTxid : 5th param, from listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', txid
    // unspentVOut : 6th param, from listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', vout
    // unspentRedeemScript : 7th param, from listunspent 1 99999999 '["48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ"]', redeemScript => maybe createMultiSig
    // privateKey : 8th param, from dumpprivkey ${addr}, => maybe createkeypair

    string rawTxHexForMultisig = createStreamPublishTx("key1", "tested by moony",
                            "a0b59e8c6f2fd144485d19632f62708f88116fb11a46411dd7d1e211ec92ce9a",
                            "a9143e45d3a48882576ad5900978303705e1a6000305871473706b6511feed9499be6fb101e0f59119d3fe15751473706b700800000000000000fffffffffbfe095c75",
                            "db84077722b74c9c9a799cf58d6c7a265f214f003b5ef15cae368a8add8d33f8", 0,
                            "5221027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b21038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd52ae",
                            "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp",
                            SamplePrivateKeyHelper()
                           );
    cout << "for multi sign : " << rawTxHexForMultisig << endl;

    string rawTxHex = createStreamPublishTx("key1", "tested by moony",
                            "a0b59e8c6f2fd144485d19632f62708f88116fb11a46411dd7d1e211ec92ce9a",
                            "76a9143ab53060d41b5fa662a2d4575a69464b5759839588ac1473706b700700000000000000ffffffff319ffb5b75",
                            "88a98467f24a3935156496283c1d06b2fe61b86b0d6276d14ad4ef6bcb25ffd5", 0,
                            "",
                            "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp",
                            SamplePrivateKeyHelper()
                           );
    cout << "for single sign : " << rawTxHex << endl;
}

int main()
{
    testPubkeyToAddr();

    testMultisigScript();

    testCreateRawSendFrom();

    return 0;
}
