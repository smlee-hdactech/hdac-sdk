#include "analyzetx.h"
#include "transaction.h"
#include <utils/streams.h>
#include <iostream>
#include <utils/utilstrencodings.h>
#include <utils/tinyformat.h>
#include <iomanip>
#include <vector>
#include <json_spirit/json_spirit.h>

using namespace std;
using namespace json_spirit;

inline std::string ValueString(const std::vector<unsigned char>& vch)
{
    if (vch.size() <= 4)
        return strprintf("%d", CScriptNum(vch, false).getint());
    else
        return HexStr(vch);
}


std::string analyzeScript(const CScript& script)
{
    opcodetype opcode;
    std::vector<unsigned char> vch;
    CScript::const_iterator pc = script.begin();

    std::string str;
    str += strprintf("%02x", script.size());
    while (pc < script.end())
    {
        if (!str.empty())
            str += " ";
        if (!script.GetOp(pc, opcode, vch))
        {
            str += "[error]";
            return str;
        }
        if (0 <= opcode && opcode <= OP_PUSHDATA4)
            str += strprintf("%02x %s", vch.size(), ValueString(vch));
        else
            str += strprintf("%s(%02x)", GetOpName(opcode), opcode);
    }

    return str;
}

#ifdef PRINT_CHECK
string analyzeVin(const CTxIn& vin)
#else
Object analyzeVin(const CTxIn& vin)
#endif
{
#ifdef PRINT_CHECK
    string str;
    str += strprintf("    prevhash=%s, prevout=%08x, \n",
                     vin.prevout.hash.ToString(),
                     vin.prevout.n);
    // TODO : coinbase
    //string strScriptSig = HexStr(vin.scriptSig);
    // TODO : maybe scirptSig-size has variable-length
    str += strprintf("    scriptSig: %s, \n", analyzeScript(vin.scriptSig));
    str += strprintf("    nSequence=%08x\n", vin.nSequence);
    return str;
#else
    Object parsedObj;
    parsedObj.push_back(Pair("prevhash", vin.prevout.hash.ToString()));
    parsedObj.push_back(Pair("prevout", strprintf("%08x", vin.prevout.n)));
    parsedObj.push_back(Pair("scriptSig", analyzeScript(vin.scriptSig)));
    parsedObj.push_back(Pair("nSequece", strprintf("%08x", vin.nSequence)));
    return parsedObj;
#endif
}

#ifdef PRINT_CHECK
string analyzeVout(const CTxOut& vout)
#else
Object analyzeVout(const CTxOut& vout)
#endif
{
    ostringstream stm;
    stm << setfill('0') << setw(16) << hex << vout.nValue;
#ifdef PRINT_CHECK
    string str;
    str += strprintf("    nValue=%s, ", stm.str());
    str += strprintf("\n    scriptPubKey: %s\n", analyzeScript(vout.scriptPubKey));
    return str;
#else
    Object parsedObj;
    parsedObj.push_back(Pair("nValue", stm.str()));
    parsedObj.push_back(Pair("scriptPubKey", analyzeScript(vout.scriptPubKey)));
    return parsedObj;
#endif
}

#ifdef PRINT_CHECK
void analyzeTx(const string& txHex)
#else
Object analyzeTx(const string& txHex)
#endif
{    
    CTransaction tx;
    vector<unsigned char> txData(ParseHex(txHex));
    CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
    try {
        ssData >> tx;
    }
    catch (const std::exception &) {
        cerr << "txHex is not transaction" << endl;
#ifdef PRINT_CHECK
        return;
#else
        return Object();
#endif
    }

#ifdef PRINT_CHECK
    cout << "raw: " << txHex << endl;

    cout << "parsed: " << endl;

    // TODO : vin-size has variable length
    std::string str;
    str += strprintf("ver=%08x, vin-count=%02x\n",
        tx.nVersion,
        tx.vin.size()
        );
#else
    Object parsedObj;
    parsedObj.push_back(Pair("version", strprintf("%08x", tx.nVersion)));
    parsedObj.push_back(Pair("vin-count", strprintf("%02x", tx.vin.size())));
#endif

    for (unsigned int i = 0; i < tx.vin.size(); i++) {
#ifdef PRINT_CHECK
        str += strprintf("vin[%d]: \n", i);
        str += analyzeVin(tx.vin[i]);
#else
        parsedObj.push_back(Pair(strprintf("vin[%d]", i), analyzeVin(tx.vin[i])));
#endif
    }

    // TODO : vout-size has variable length
#ifdef PRINT_CHECK
    str += strprintf("vout-count=%02x\n", tx.vout.size());
#else
    parsedObj.push_back(Pair("vout-count", strprintf("%02x", tx.vout.size())));
#endif

    for (unsigned int i = 0; i < tx.vout.size(); i++) {
#ifdef PRINT_CHECK
        str += strprintf("vout[%d]: \n", i);
        str += analyzeVout(tx.vout[i]);
#else
        parsedObj.push_back(Pair(strprintf("vout[%d]", i), analyzeVout(tx.vout[i])));
#endif
    }

#ifdef PRINT_CHECK
    str += strprintf("nLockTime=%08x", tx.nLockTime);
    cout << str << endl;
    //cout << tx.ToString();
#else
    parsedObj.push_back(Pair("nLockTime", strprintf("%08x", tx.nLockTime)));
    return parsedObj;
#endif
}

