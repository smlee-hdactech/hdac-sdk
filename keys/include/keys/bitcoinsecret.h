#ifndef BITCOINSECRET_H
#define BITCOINSECRET_H

#include <structs/base58.h>
#include "key.h"
#include "keyshelper.h"
/**
 * A base58-encoded secret key
 */
class CBitcoinSecret : public CBase58Data
{
public:
    void SetKey(const CKey& vchSecret);
    CKey GetKey();
    bool IsValid() const;
    bool SetString(const char* pszSecret);
    bool SetString(const std::string& strSecret);

    CBitcoinSecret(const CKey& vchSecret, const IPrivateKeyHelper& helper) :
        CBase58Data(helper.addrChecksumValue()),
        _privkeyPrefix(helper.privkeyPrefix())   {
        SetKey(vchSecret);
    }

    CBitcoinSecret(const IPrivateKeyHelper& helper) :
        CBase58Data(helper.addrChecksumValue()),
        _privkeyPrefix(helper.privkeyPrefix())    {}

private:
    const std::vector<unsigned char> _privkeyPrefix;
};

#endif // BITCOINSECRET_H
