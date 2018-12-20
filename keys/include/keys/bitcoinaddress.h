#ifndef BITCOINADDRESS_H
#define BITCOINADDRESS_H

#include <structs/base58.h>
#include "pubkey.h"
#include <script/standard.h>
#include <utils/utility.h>
#include "keyshelper.h"

/** base58-encoded Bitcoin addresses.
 * Public-key-hash-addresses have version 0 (or 111 testnet).
 * The data vector contains RIPEMD160(SHA256(pubkey)), where pubkey is the serialized public key.
 * Script-hash-addresses have version 5 (or 196 testnet).
 * The data vector contains RIPEMD160(SHA256(cscript)), where cscript is the serialized redemption script.
 */
class CBitcoinAddress : public CBase58Data {
public:
    //bool Set(const CKeyID &id, const std::vector<unsigned char> &pubkeyPrefix);
    bool Set(const CKeyID &id);
    bool Set(const CKeyID &id,const std::vector<unsigned char>& vchVersion);
    bool Set(const CScriptID &id);
    bool Set(const CTxDestination &dest);
    bool IsValid() const;
    //bool IsValid(const CChainParams &params) const;
    bool IsValid(const std::vector<unsigned char>& pubkeyPrefix,
                 const std::vector<unsigned char>& scriptPrefix) const;

    CBitcoinAddress(const IWalletAddrHelper &helper) :
        CBase58Data(helper.addrChecksumValue()),
        _pubkeyPrefix(helper.pubkeyAddrPrefix()),
        _scriptPrefix(helper.scriptAddrPrefix()) {}

    CBitcoinAddress(const CTxDestination &dest,
        const IWalletAddrHelper &helper) :
        CBitcoinAddress(helper) {
        Set(dest);
    }

    CBitcoinAddress(const std::string& strAddress,
                    const IWalletAddrHelper &helper) :
        CBitcoinAddress(helper) {
        SetString(strAddress, helper.pubkeyAddrPrefix().size());
    }

    CBitcoinAddress(const char* pszAddress,
                    const IWalletAddrHelper &helper) :
        CBitcoinAddress(helper) {
        SetString(pszAddress);
    }

    CTxDestination Get() const;
    bool GetKeyID(CKeyID &keyID) const;
    bool GetScriptID(CScriptID &scriptID) const;
    bool IsScript() const;

    const std::vector<unsigned char> _pubkeyPrefix;
    const std::vector<unsigned char> _scriptPrefix;
};

#endif // BITCOINADDRESS_H
