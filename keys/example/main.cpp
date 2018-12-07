#include <keys/bitcoinaddress.h>
#include <iostream>
#include <utils/utilsfront.h>

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

void testPubkeyToAddr()
{
    CPubKey pubkey(ParseHex("027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b"));
    cout << "pubKey: " << HexStr(pubkey) << endl;

    //cout << hex << checksum << endl;
    CBitcoinAddress addr(pubkey.GetID(), SampleWalletAddrHelper());

    cout << "address : " << addr.ToString() << endl;
}

int main()
{
    testPubkeyToAddr();
    return 0;
}
