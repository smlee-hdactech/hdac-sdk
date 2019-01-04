#ifndef HS_HELPERS_H
#define HS_HELPERS_H

#include <memory>
#include <keys/keyshelper.h>
#include <map>

class RpcClient;
class KeysHelperWithRpc {
public:
    KeysHelperWithRpc(const RpcClient& client);

    IWalletAddrHelper& addrHelper();

    IPrivateKeyHelper& privHelper();

private:
    std::map<std::string, std::string> _resultMap;

    class WalletAddrHelper : public IWalletAddrHelper {
    public:
        WalletAddrHelper(const std::map<std::string, std::string> &result) :
            _resultMap(result)  { }

        const std::vector<unsigned char> pubkeyAddrPrefix() const override;
        const std::vector<unsigned char> scriptAddrPrefix() const override;

        int32_t addrChecksumValue() const override;
    private:
        const std::map<std::string, std::string> & _resultMap;

    };
    std::unique_ptr<WalletAddrHelper> _addrHelper;

    class PrivateKeyHelper : public IPrivateKeyHelper {
    public:
        PrivateKeyHelper(const std::map<std::string, std::string> &result) :
            _resultMap(result)  { }

        const std::vector<unsigned char> privkeyPrefix() const override;

        int32_t addrChecksumValue() const override;
    private:
        const std::map<std::string, std::string> &_resultMap;
    };
    std::unique_ptr<PrivateKeyHelper> _privHelper;
};

#endif // HS_HELPERS_H
