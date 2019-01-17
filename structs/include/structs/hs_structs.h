#ifndef HS_STRUCTS_H
#define HS_STRUCTS_H

#include <vector>
#include <string>

std::vector<unsigned char> obtainHash(const std::string& input);
std::vector<unsigned char> hashFromFile(const std::string& filename);

#endif // HS_STRUCTS_H
