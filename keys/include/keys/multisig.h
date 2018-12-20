#ifndef MULTISIG_H
#define MULTISIG_H

#include <vector>
#include <string>
#include <script/script.h>
#include <script/standard.h>

class CPubKey;
CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys);
CScript GetScriptForDestination(const CTxDestination& dest);
CScript createMultisigRedeemScript(int nRequired, const std::vector<std::string>& keys,
                                   bool onlyAcceptStdTx = true, unsigned int maxStdElemSize = 8192);

struct MultisigAddrInfo
{
    std::string addr;
    std::string redeemScript;
};

class IWalletAddrHelper;
void createMultisigInfo(const std::vector<std::string>& pubkeys, int required, const IWalletAddrHelper& addrHelper, MultisigAddrInfo& info);

#endif // MULTISIG_H
