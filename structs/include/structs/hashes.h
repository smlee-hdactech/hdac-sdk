#ifndef HASHES_H
#define HASHES_H

#include "hashes_global.h"
#include <string>
#include <functional>
#include <crypto/sha256.h>

//HASHESSHARED_EXPORT void hashFromFile(const std::string& filename);
HASHESSHARED_EXPORT void obtainHash(const std::string& input, std::function<void(unsigned char hash[CSHA256::OUTPUT_SIZE])> callback);
HASHESSHARED_EXPORT void compareHashValue(unsigned char hash[CSHA256::OUTPUT_SIZE], const std::string& correct);

#endif // HASHES_H
