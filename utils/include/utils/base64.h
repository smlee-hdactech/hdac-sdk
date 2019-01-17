#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>
// TODO : global headers -> only one.
#include "strcodeclib_global.h"

std::vector<unsigned char> DecodeBase64(const char* p, bool* pfInvalid = NULL);
std::string DecodeBase64(const std::string& str);
std::string EncodeBase64(const unsigned char* pch, size_t len);
std::string EncodeBase64(const std::string& str);

#endif // BASE64_H
