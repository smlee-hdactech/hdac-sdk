#ifndef RPCCALLER_H
#define RPCCALLER_H

#include "rpccaller_global.h"
#include <json_spirit/json_spirit.h>

RPCCALLERSHARED_EXPORT void ShowResultWithRPC(const std::string &method, const json_spirit::Array &params);


#endif // RPCCALLER_H
