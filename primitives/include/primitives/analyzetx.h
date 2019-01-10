#ifndef ANALYZETX_H
#define ANALYZETX_H

#include <string>
#include <json_spirit/json_spirit.h>

//#define PRINT_CHECK

#ifdef PRINT_CHECK
void analyzeTx(const std::string& txHex);
#else
json_spirit::Object analyzeTx(const std::string& txHex);
#endif

#endif  //ANALYZETX_H
