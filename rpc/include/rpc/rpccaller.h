#ifndef RPCCALLER_H
#define RPCCALLER_H

#include "rpccaller_global.h"
#include <json_spirit/json_spirit.h>

RPCCALLERSHARED_EXPORT int error(const json_spirit::Object& reply, std::string &strResult);
RPCCALLERSHARED_EXPORT int result(const json_spirit::Object& reply, std::string &strResult);

#endif // RPCCALLER_H
