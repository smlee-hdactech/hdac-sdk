#ifndef KEYS_STANDARD_H
#define KEYS_STANDARD_H

#include <vector>
#include <script/standard.h>

class CScript;

enum txnouttype
{
    TX_NONSTANDARD,
    // 'standard' transaction types:
    TX_PUBKEY,
    TX_PUBKEYHASH,
    TX_SCRIPTHASH,
    TX_MULTISIG,
    TX_NULL_DATA,
};

bool TemplateSolver(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<std::vector<unsigned char> >& vSolutionsRet);
bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<CTxDestination>& addressRet, int& nRequiredRet,std::vector<std::vector<unsigned char> >* lpvSolutionsRet = NULL);

typedef std::vector<unsigned char> valtype;

#endif // KEYS_STANDARD_H
