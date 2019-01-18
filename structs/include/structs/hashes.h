#ifndef HASHES_H
#define HASHES_H

#include "hashes_global.h"
#include <string>
#include <functional>
#include <crypto/sha256.h>

//HASHESSHARED_EXPORT void hashFromFile(const std::string& filename);
void obtainHash(const std::string& input, std::function<void(unsigned char hash[CSHA256::OUTPUT_SIZE])> callback);
void compareHashValue(unsigned char hash[CSHA256::OUTPUT_SIZE], const std::string& correct);

#endif // HASHES_H
