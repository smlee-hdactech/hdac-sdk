#include "base58.h"
#include <cassert>
#include <cstring>
#include "hash.h"
#include <utils/utility.h>

/** All alphanumeric characters except for "0", "I", "O", and "l" */
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch)
{
    // Skip leading spaces.
    while (*psz && isspace(*psz))
        psz++;
    // Skip and count leading '1's.
    int zeroes = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    std::vector<unsigned char> b256(strlen(psz) * 733 / 1000 + 1); // log(58) / log(256), rounded up.
    // Process the characters.
    while (*psz && !isspace(*psz)) {
        // Decode base58 character
        const char* ch = strchr(pszBase58, *psz);
        if (ch == NULL)
            return false;
        // Apply "b256 = b256 * 58 + ch".
        int carry = ch - pszBase58;
        for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); it != b256.rend(); it++) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        psz++;
    }
    // Skip trailing spaces.
    while (isspace(*psz))
        psz++;
    if (*psz != 0)
        return false;
    // Skip leading zeroes in b256.
    std::vector<unsigned char>::iterator it = b256.begin();
    while (it != b256.end() && *it == 0)
        it++;
    // Copy result into output vector.
    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end())
        vch.push_back(*(it++));
    return true;
}

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
    // Skip & count leading zeroes.
    int zeroes = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    std::vector<unsigned char> b58((pend - pbegin) * 138 / 100 + 1); // log(256) / log(58), rounded up.
    // Process the bytes.
    while (pbegin != pend) {
        int carry = *pbegin;
        // Apply "b58 = b58 * 256 + ch".
        for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); it != b58.rend(); it++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }
        assert(carry == 0);
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    std::vector<unsigned char>::iterator it = b58.begin();
    while (it != b58.end() && *it == 0)
        it++;
    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
    return str;
}

std::string EncodeBase58(const std::vector<unsigned char>& vch)
{
    return EncodeBase58(&vch[0], &vch[0] + vch.size());
}

bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet)
{
    return DecodeBase58(str.c_str(), vchRet);
}

std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn, int32_t addressChecksumValue)
{
    using namespace std;
    // add 4-byte hash check to the end
    std::vector<unsigned char> vch(vchIn);
    uint256 hash = Hash(vch.begin(), vch.end());

    int32_t checksum=(int32_t)mc_GetLE(&hash,4);
    //checksum ^= (int32_t)mc_gState->m_NetworkParams->GetInt64Param("addresschecksumvalue");
    checksum ^= addressChecksumValue;
    vch.insert(vch.end(), (unsigned char*)&checksum, (unsigned char*)&checksum + 4);

    return EncodeBase58(vch);
}

bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet, int32_t addressChecksumValue)
{
    if (!DecodeBase58(psz, vchRet) ||
        (vchRet.size() < 4)) {
        vchRet.clear();
        return false;
    }
    // re-calculate the checksum, insure it matches the included 4-byte checksum
    uint256 hash = Hash(vchRet.begin(), vchRet.end() - 4);

    int32_t checksum=(int32_t)mc_GetLE(&hash,4);
    //checksum ^= (int32_t)mc_gState->m_NetworkParams->GetInt64Param("addresschecksumvalue");
    checksum ^= addressChecksumValue;

    if (memcmp((unsigned char*)&checksum, &vchRet.end()[-4], 4) != 0) {
        vchRet.clear();
        return false;
    }
    vchRet.resize(vchRet.size() - 4);
    return true;
}

bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet, int32_t addressChecksumValue)
{
    return DecodeBase58Check(str.c_str(), vchRet, addressChecksumValue);
}

CBase58Data::CBase58Data(int32_t checksumValue) :
    _checksumValue(checksumValue)
{
    vchVersion.clear();
    vchData.clear();
}

void CBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const void* pdata, size_t nSize)
{
    vchVersion = vchVersionIn;
    vchData.resize(nSize);
    if (!vchData.empty())
        memcpy(&vchData[0], pdata, nSize);
}

void CBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const unsigned char* pbegin, const unsigned char* pend)
{
    SetData(vchVersionIn, (void*)pbegin, pend - pbegin);
}

bool CBase58Data::SetString(const char* psz, unsigned int nVersionBytes)
{
    std::vector<unsigned char> vchTemp;

    assert(_checksumValue != 0);
    bool rc58 = DecodeBase58Check(psz, vchTemp, _checksumValue);
    if ((!rc58) || (vchTemp.size() < nVersionBytes)) {
        vchData.clear();
        vchVersion.clear();
        return false;
    }

    int shift=(vchTemp.size() - nVersionBytes) / nVersionBytes;
    vchVersion.resize(nVersionBytes);
    vchData.resize(vchTemp.size() - nVersionBytes);
    for(int i=0;i<(int)nVersionBytes;i++)
    {
        vchVersion[i]=vchTemp[i*(shift+1)];
        int size=shift;
        if(i == (int)(nVersionBytes-1))
        {
            size=(vchTemp.size() - nVersionBytes)-i*shift;
        }
        memcpy(&vchData[i*shift],&vchTemp[i*(shift+1)+1],size);
    }

    OPENSSL_cleanse(&vchTemp[0], vchTemp.size());
    return true;
}

bool CBase58Data::SetString(const std::string& str, unsigned int pubkeyAddressSize)
{
    //return SetString(str.c_str(),Params().Base58Prefix(CChainParams::PUBKEY_ADDRESS).size());
    return SetString(str.c_str(), pubkeyAddressSize);
}

std::string CBase58Data::ToString() const
{
    std::vector<unsigned char> vch;
    int nVersionBytes=vchVersion.size();

    int shift=vchData.size() / nVersionBytes;
    vch.resize(vchData.size() + nVersionBytes);
    for(int i=0;i<nVersionBytes;i++)
    {
        vch[i*(shift+1)]=vchVersion[i];
        int size=shift;
        if(i == nVersionBytes-1)
        {
            size=vchData.size()-i*shift;
        }
        memcpy(&vch[i*(shift+1)+1],&vchData[i*shift],size);
    }

    return EncodeBase58Check(vch, _checksumValue);
}

int CBase58Data::CompareTo(const CBase58Data& b58) const
{
    if (vchVersion < b58.vchVersion)
        return -1;
    if (vchVersion > b58.vchVersion)
        return 1;
    if (vchData < b58.vchData)
        return -1;
    if (vchData > b58.vchData)
        return 1;
    return 0;
}
