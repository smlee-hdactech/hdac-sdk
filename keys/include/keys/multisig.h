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

#endif // MULTISIG_H
