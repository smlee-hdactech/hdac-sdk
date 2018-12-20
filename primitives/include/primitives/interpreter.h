#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <structs/uint256.h>

class CScript;
class CTransaction;

/** Signature hash types/flags */
enum
{
    SIGHASH_ALL = 1,
    SIGHASH_NONE = 2,
    SIGHASH_SINGLE = 3,
    SIGHASH_ANYONECANPAY = 0x80,
};

uint256 SignatureHash(const CScript &scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

#endif // INTERPRETER_H
