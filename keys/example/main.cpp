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
    // "027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b","038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd"
    vector<string> pubkeys = {
        "027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b",
        "038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd"
    };
    CScript script = createMultisigRedeemScript(2, pubkeys);
    cout << "MultisigRedeemScript: ";
    cout << HexStr(script) << endl;
    CScriptID scriptID(script);
    cout << "MultisigRedeemScript ID: ";
    cout << HexStr(scriptID) << endl;

#if 0 // how to handle wallet
    pwalletMain->AddCScript(inner);

    pwalletMain->SetAddressBook(innerID, strAccount, "send");

    if(mc_gState->m_WalletMode & MC_WMD_ADDRESS_TXS)
    {
        mc_TxEntity entity;
        entity.Zero();
        memcpy(entity.m_EntityID,&innerID,MC_TDB_ENTITY_ID_SIZE);
        entity.m_EntityType=MC_TET_SCRIPT_ADDRESS | MC_TET_CHAINPOS;
        pwalletTxsMain->AddEntity(&entity,MC_EFL_NOT_IN_SYNC);
        entity.m_EntityType=MC_TET_SCRIPT_ADDRESS | MC_TET_TIMERECEIVED;
        pwalletTxsMain->AddEntity(&entity,MC_EFL_NOT_IN_SYNC);
    }
#endif
    CScript outer = GetScriptForDestination(scriptID);
    cout << "outer: " << HexStr(outer) << endl;

#if 0
    if(IsMine(*pwalletMain, outer) == ISMINE_NO)
    {
        if (!pwalletMain->HaveWatchOnly(outer))
        {
            if (!pwalletMain->AddWatchOnly(outer))
                throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
        }
    }
#endif

    //return CBitcoinAddress(innerID).ToString();
    cout << "multisig addr: ";
    cout << CBitcoinAddress(scriptID, SampleWalletAddrHelper()).ToString() << endl;
}

void testCreateRawSendFrom()
{
    createMultisigStreamTx("48R3XwXEYBtbq74WrRzRV4UeWugTPUSZmG1deQ",
                           "stream9", "key1", "tested by moony",
                           "a0b59e8c6f2fd144485d19632f62708f88116fb11a46411dd7d1e211ec92ce9a",
                           "a9143e45d3a48882576ad5900978303705e1a6000305871473706b6511feed9499be6fb101e0f59119d3fe15751473706b700800000000000000fffffffffbfe095c75",
                           "db84077722b74c9c9a799cf58d6c7a265f214f003b5ef15cae368a8add8d33f8", 0,
                           "5221027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b21038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd52ae",
                           "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp"
                           );
}

int main()
{
    testPubkeyToAddr();

    testMultisigScript();

    testCreateRawSendFrom();

    return 0;
}
