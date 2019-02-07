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

std::string analyzeDetail(const std::vector<unsigned char>& vch)
{
    string str = strprintf("%s(%s) ", string(vch.begin(), vch.begin()+4), HexStr(vch.begin(), vch.begin()+4));

    switch(vch[3]) {
    case 'g':   // New issuance quantity
        break;
    case 'n':   // New issuance metadata, Stream creation outputs
        break;
    case 'q': {  // Per-output asset metadata
        const int64_t* qty = reinterpret_cast<const int64_t*>(&vch[4+16]);
        return str + "ASSET-ID(" + HexStr(vch.begin()+4, vch.begin()+4+16) + ") " + "QTY:"+ to_string(*qty) + "(" + HexStr(vch.begin()+4+16, vch.end()) + ")";
    }
    case 'o':   // Follow-on issuance quantity
        break;
    case 'e':   // Follow-on issuance metadata::asset-identifier, Stream item outputs::stream-identifier
        return str + "ASSET|STREAM-ID(" + HexStr(vch.begin()+4, vch.end()) + ")";
    case 'u':   // Follow-on issuance metadata:issue-details
        return str + "ASSET-TYPE(" + HexStr(vch.begin()+4, vch.begin() + 5) + ") " + "ASSET-PROP(" + HexStr(vch.begin()+5, vch.end()) + ")";
    case 'k':   // Stream item outputs::item-key
        return str + "STREAM-KEY:" + string(vch.begin()+4, vch.end()) + "(" + HexStr(vch.begin()+4, vch.end()) + ")";
    }

    return str + HexStr(vch.begin()+4, vch.end());
}

bool isSpeciaMeta(const std::vector<unsigned char>& vch)
{
    if (string(&vch[0], &vch[3]) == "spk") {
        return true;
    }
    return false;
}

inline std::string ValueString(const std::vector<unsigned char>& vch)
{
    if (vch.size() <= 4)
        return strprintf("%d", CScriptNum(vch, false).getint());
    else if (isSpeciaMeta(vch)) {
        return analyzeDetail(vch);
    }
    else {
        return HexStr(vch);
    }
}


std::string analyzeScript(const CScript& script, function<string(const vector<unsigned char>&, bool)> convertAddr = nullptr)
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
		if (0 <= opcode && opcode <= OP_PUSHDATA4) {
			if (vch.size() == 20) {
				if (convertAddr) {
					str += strprintf("%02x %s(%s)", vch.size(), convertAddr(vch, script.IsPayToScriptHash()), ValueString(vch));
				}
				else {
					str += strprintf("%02x %s", vch.size(), ValueString(vch));
				}
			}
			else {
				if (opcode == 0 && vch.size() == 0) {
					str += strprintf("%s(%02x)", GetOpName(opcode), opcode);
				}
				else {					
					if (vch[0] >= OP_1 && vch[1] <= OP_16) {
						CScript scr(vch.begin(), vch.end());
						str += analyzeScript(scr);
					}
					else {
						str += strprintf("%02x %s", vch.size(), ValueString(vch));
					}
				}
			}
		}
		else {
			str += strprintf("%s(%02x)", GetOpName(opcode), opcode);
		}
    }

    return str;
}


Object analyzeVin(const CTxIn& vin)
{
    Object parsedObj;
    parsedObj.push_back(Pair("prevhash", vin.prevout.hash.ToString()));
    parsedObj.push_back(Pair("prevout", strprintf("%08x", vin.prevout.n)));
    parsedObj.push_back(Pair("scriptSig", analyzeScript(vin.scriptSig)));
    parsedObj.push_back(Pair("nSequece", strprintf("%08x", vin.nSequence)));
    return parsedObj;
}

Object analyzeVout(const CTxOut& vout, function<string(const vector<unsigned char>&, bool)> convertAddr = nullptr)
{
    ostringstream stm;
    stm << setfill('0') << setw(16) << hex << vout.nValue;

    Object parsedObj;
    parsedObj.push_back(Pair("nValue", stm.str()));
    parsedObj.push_back(Pair("scriptPubKey", analyzeScript(vout.scriptPubKey, convertAddr)));
	parsedObj.push_back(Pair("isScriptHash", vout.scriptPubKey.IsPayToScriptHash()));
    return parsedObj;
}

/**
 *
 * @brief raw 트랜잭션 문자열을 분석한다.
 * @details raw 트랜잭션의 hex값으로 표시된 값을 파싱하여 세부값들을 JSON 객체로 만든다.
 * @param txHex raw 트랜잭션의 hex값으로 표시된 값
 *
 * @return 분석된 JSON 객체
 *
 */
Object analyzeTx(const string& txHex, function<string(const vector<unsigned char>&, bool)> convertAddr)
{    
    CTransaction tx;
    vector<unsigned char> txData(ParseHex(txHex));
    CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
    try {
        ssData >> tx;
    }
    catch (const std::exception &) {
        cerr << "txHex is not transaction" << endl;
        return Object();
    }

    Object parsedObj;
    parsedObj.push_back(Pair("version", strprintf("%08x", tx.nVersion)));
    parsedObj.push_back(Pair("vin-count", strprintf("%02x", tx.vin.size())));

    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        parsedObj.push_back(Pair(strprintf("vin[%d]", i), analyzeVin(tx.vin[i])));
    }

    // TODO : vout-size has variable length
    parsedObj.push_back(Pair("vout-count", strprintf("%02x", tx.vout.size())));

    for (unsigned int i = 0; i < tx.vout.size(); i++) {
        parsedObj.push_back(Pair(strprintf("vout[%d]", i), analyzeVout(tx.vout[i], convertAddr)));
    }

    parsedObj.push_back(Pair("nLockTime", strprintf("%08x", tx.nLockTime)));
    return parsedObj;
}

