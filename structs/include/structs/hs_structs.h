/**
* @file		hs_structs.h
* @date		2019-01-17
* @author	HDAC Technology Inc.
*
* @brief	hs_structs 헤더 파일.
*/

#ifndef HS_STRUCTS_H
#define HS_STRUCTS_H

#include <vector>
#include <string>

/// 스트링 문자열을 sha256 해시로 변환 시켜 준다.
std::vector<unsigned char> obtainHash(const std::string& input);
/// 파일을 sha256 해시로 변환 시켜 준다.
std::vector<unsigned char> hashFromFile(const std::string& filename);

#endif // HS_STRUCTS_H
