/**
* @file		hs_structs.cpp 
* @date 	2019-01-17
* @author	HDAC Technology Inc.
*
* @brief	hs_structs 소스 파일. 
*/

#include "hs_structs.h"
#include <crypto/sha256.h>
#include <fstream>
#include <iostream>
#include <utils/base64.h>

using namespace std;

/**
 *
 * @brief input 스트링 문자열에 대하여 vector 형 hash 를 만들어 준다.
 * @details input 으로 받은 스트링 문자열을 sha256 해시로 변환하여 반환 한다.
 * @param input 해시 할 문자열
 *
 * @return sha256 으로 만든 vertor 형 해시 값
 *
 */
vector<unsigned char> obtainHash(const string& input)
{
    CSHA256 sha;
    unsigned char hash[CSHA256::OUTPUT_SIZE];
    sha.Write(reinterpret_cast<const unsigned char*>(input.c_str()), input.length()).Finalize(hash);
    return vector<unsigned char>(&hash[0], &hash[CSHA256::OUTPUT_SIZE]);
}

/**
 *
 * @brief 파일에 대하여 vector 형 hash 를 만들어 준다.
 * @details input 으로 받은 파일을 sha256 해시로 변환하여 반환 한다.
 * @param filename 해시 할 파일
 *
 * @return sha256 으로 만든 vertor 형 해시 값
 *
 */
vector<unsigned char> hashFromFile(const string& filename)
{
    ifstream is(filename, ifstream::in | ifstream::binary);
    if (is.is_open() == false)  {
        cerr << "file not open" << endl;
        return vector<unsigned char>();
    }
    is.seekg(0, ios::end);
    int size = (int)is.tellg();
    string readStr;
    readStr.resize(size);
    is.seekg(0, ios::beg);
    is.read(&readStr[0], size);
    string toBase64 = EncodeBase64(readStr);
    return obtainHash(toBase64);
}
