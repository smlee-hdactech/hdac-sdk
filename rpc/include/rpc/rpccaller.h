#ifndef RPCCALLER_H
#define RPCCALLER_H

#include "rpccaller_global.h"
#include <json_spirit/json_spirit.h>

int error(const json_spirit::Object& reply, std::string &strResult);
int result(const json_spirit::Object& reply, std::string &strResult);

#endif // RPCCALLER_H
