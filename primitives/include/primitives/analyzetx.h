#ifndef ANALYZETX_H
#define ANALYZETX_H

#include <string>
#include <json_spirit/json_spirit.h>

/// raw 트랜잭션 문자열을 분석한다.
json_spirit::Object analyzeTx(const std::string& txHex);

#endif  //ANALYZETX_H
