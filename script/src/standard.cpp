#include "standard.h"
#include "script.h"
#include <structs/hash.h>

using namespace std;

CScriptID::CScriptID(const CScript& in) : uint160(Hash160(in.begin(), in.end())) {}


