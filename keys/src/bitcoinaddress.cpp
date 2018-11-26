#include "bitcoinaddress.h"

namespace
{
class CBitcoinAddressVisitor : public boost::static_visitor<bool>
{
private:
    CBitcoinAddress* addr;

public:
    CBitcoinAddressVisitor(CBitcoinAddress* addrIn) : addr(addrIn) {}

    bool operator()(const CKeyID& id) const { return addr->Set(id); }
    bool operator()(const CScriptID& id) const { return addr->Set(id); }
    bool operator()(const CNoDestination& no) const { return false; }
};

} // anon namespace

bool CBitcoinAddress::Set(const CKeyID &id,const std::vector<unsigned char>& vchV)
{
    SetData(vchV, &id, 20);
    return true;
}


bool CBitcoinAddress::Set(const CKeyID& id)
{
    assert(_pubkeyPrefix.empty() == false);
    //SetData(Params().Base58Prefix(CChainParams::PUBKEY_ADDRESS), &id, 20);
    SetData(_pubkeyPrefix, &id, 20);
    return true;
}

bool CBitcoinAddress::Set(const CScriptID& id)
{
    //SetData(Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS), &id, 20);
    assert(_scriptPrefix.empty() == false);
    SetData(_scriptPrefix, &id, 20);
    return true;
}

bool CBitcoinAddress::Set(const CTxDestination& dest)
{
    return boost::apply_visitor(CBitcoinAddressVisitor(this), dest);
}

bool CBitcoinAddress::IsValid() const
{
    assert(_pubkeyPrefix.empty() == false && _scriptPrefix.empty() == false);
    return IsValid(_pubkeyPrefix, _scriptPrefix);
    //return IsValid(Params());
}

//bool CBitcoinAddress::IsValid(const CChainParams& params) const
bool CBitcoinAddress::IsValid(const std::vector<unsigned char>& pubkeyPrefix,
                              const std::vector<unsigned char>& scriptPrefix) const
{

    bool fCorrectSize = vchData.size() == 20;
    //bool fKnownVersion = vchVersion == params.Base58Prefix(CChainParams::PUBKEY_ADDRESS) ||
    //                     vchVersion == params.Base58Prefix(CChainParams::SCRIPT_ADDRESS);
    bool fKnownVersion = vchVersion == pubkeyPrefix || vchVersion == scriptPrefix;
    return fCorrectSize && fKnownVersion;
}

CTxDestination CBitcoinAddress::Get() const
{
    if (!IsValid())
        return CNoDestination();
    uint160 id;
    memcpy(&id, &vchData[0], 20);
    //if (vchVersion == Params().Base58Prefix(CChainParams::PUBKEY_ADDRESS))
    if (vchVersion == _pubkeyPrefix)
        return CKeyID(id);
    //else if (vchVersion == Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS))
    if (vchVersion == _scriptPrefix)
        return CScriptID(id);
    else
        return CNoDestination();
}

bool CBitcoinAddress::GetKeyID(CKeyID& keyID) const
{
    assert(_pubkeyPrefix.empty() == false);
    //if (!IsValid() || vchVersion != Params().Base58Prefix(CChainParams::PUBKEY_ADDRESS))
    if (!IsValid() || vchVersion != _pubkeyPrefix)
        return false;
    uint160 id;
    memcpy(&id, &vchData[0], 20);
    keyID = CKeyID(id);
    return true;
}

bool CBitcoinAddress::GetScriptID(CScriptID& scriptID) const
{
    assert(_scriptPrefix.empty() == false);
    //if (!IsValid() || vchVersion != Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS))
    if (!IsValid() || vchVersion != _scriptPrefix)
        return false;
    uint160 id;
    memcpy(&id, &vchData[0], 20);
    scriptID = CScriptID(id);
    return true;
}

bool CBitcoinAddress::IsScript() const
{
    assert(_scriptPrefix.empty() == false);
    //return IsValid() && vchVersion == Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS);
    return IsValid() && vchVersion == _scriptPrefix;
}
