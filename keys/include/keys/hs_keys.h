#ifndef HS_KEYS_H
#define HS_KEYS_H

#include <string>
#include <memory>

struct KeyPairs {
    std::string privateKey;
    std::string pubkey;
    std::string pubkeyHash;
    std::string walletAddr;
};

class IPrivateKeyHelper;
class IWalletAddrHelper;
KeyPairs createKeyPairs(const IPrivateKeyHelper &privateHelper, const IWalletAddrHelper &addrHelper);

class IWalletAddrHelper;
class IPrivateKeyHelper;
std::string createStreamPublishTx(const std::string& streamKey, const std::string& streamItem,
                                  const std::string &createTxid,
                                  const std::string &unspentScriptPubKey, const std::string &unspentTxid, uint32_t unspentVOut,
                                  const std::string &unspentRedeemScript, const std::string &privateKey,
                                  const IPrivateKeyHelper &helper);

std::string createAssetSendTx(const std::string& toAddr, double quantity,
                         const std::string& issueTxid, int multiple,
                         const std::string& unspentScriptPubKey, const std::string& unspentTxid, uint32_t unspentVOut,
                         double unspentQty, const std::string &unspentRedeemScript,
                         const std::string& privateKey, const IPrivateKeyHelper& privateHelper, const IWalletAddrHelper &walletHelper);

std::string walletAddrFromPubKey(const std::string& pubkeyStr, const IWalletAddrHelper& addrHelpler);

bool verifymessage(std::string strAddress, std::string strSign, std::string strMessage, const IWalletAddrHelper &addrHelper);
std::string signmessage(std::string strAddress, std::string strMessage, 
						const IPrivateKeyHelper &privateHelper, const IWalletAddrHelper &addrHelper);

#endif // HS_KEYS_H
