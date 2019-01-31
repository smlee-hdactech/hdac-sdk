#ifndef ANALYZETX_H
#define ANALYZETX_H

#include <string>
#include <json_spirit/json_spirit.h>
#include <functional>

/// raw 트랜잭션 문자열을 분석한다.
json_spirit::Object analyzeTx(const std::string& txHex, std::function<std::string(const std::vector<unsigned char>&, bool)> convertAddr = nullptr);

#endif  //ANALYZETX_H
