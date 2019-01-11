#ifndef ANALYZETX_H
#define ANALYZETX_H

#include <string>
#include <json_spirit/json_spirit.h>

json_spirit::Object analyzeTx(const std::string& txHex);

#endif  //ANALYZETX_H
