#ifndef KEYSHELPER_H
#define KEYSHELPER_H

#include <cstdint>
#include <vector>

class IWalletAddrPrefixHelper
{
public:
    virtual const std::vector<unsigned char> pubkeyAddrPrefix() const = 0;
    virtual const std::vector<unsigned char> scriptAddrPrefix() const = 0;
};

class IPrivateKeyPrefixHelper
{
public:
    virtual const std::vector<unsigned char> privkeyPrefix() const = 0;
};

class IAddrChecksumValueHelper
{
public:
    virtual int32_t addrChecksumValue() const = 0;
};

class IWalletAddrHelper : public IWalletAddrPrefixHelper, public IAddrChecksumValueHelper {

};

class IPrivateKeyHelper : public IPrivateKeyPrefixHelper, public IAddrChecksumValueHelper {

};

#endif // KEYSHELPER_H
