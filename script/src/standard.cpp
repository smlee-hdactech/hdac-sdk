#include "standard.h"
#include "script.h"
#include <structs/hash.h>

CScriptID::CScriptID(const CScript& in) : uint160(Hash160(in.begin(), in.end())) {}
