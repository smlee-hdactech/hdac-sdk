#ifndef SCRIPT_STANDARD_H
#define SCRIPT_STANDARD_H

#include <structs/uint256.h>
#include <boost/variant.hpp>

/** A reference to a CKey: the Hash160 of its serialized public key */
class CKeyID : public uint160
{
public:
    CKeyID() : uint160(0) {}
    CKeyID(const uint160& in) : uint160(in) {}
};

class CScript;
/** A reference to a CScript: the Hash160 of its serialization (see script.h) */
class CScriptID : public uint160
{
public:
    CScriptID() : uint160(0) {}
    CScriptID(const CScript& in);
    CScriptID(const uint160& in) : uint160(in) {}
};

class CNoDestination {
public:
    friend bool operator==(const CNoDestination &a, const CNoDestination &b) { return true; }
    friend bool operator<(const CNoDestination &a, const CNoDestination &b) { return true; }
};

/**
 * A txout script template with a specific destination. It is either:
 *  * CNoDestination: no destination set
 *  * CKeyID: TX_PUBKEYHASH destination
 *  * CScriptID: TX_SCRIPTHASH destination
 *  A CTxDestination is the internal data type encoded in a CBitcoinAddress
 */
typedef boost::variant<CNoDestination, CKeyID, CScriptID> CTxDestination;


#endif  //SCRIPT_STANDARD_H
