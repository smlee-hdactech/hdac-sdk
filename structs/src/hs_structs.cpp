#include "hs_structs.h"
#include <crypto/sha256.h>
#include <fstream>
#include <iostream>
#include <utils/base64.h>

using namespace std;

vector<unsigned char> obtainHash(const string& input)
{
    CSHA256 sha;
    unsigned char hash[CSHA256::OUTPUT_SIZE];
    sha.Write(reinterpret_cast<const unsigned char*>(input.c_str()), input.length()).Finalize(hash);
    return vector<unsigned char>(&hash[0], &hash[CSHA256::OUTPUT_SIZE]);
}

vector<unsigned char> hashFromFile(const string& filename)
{
    ifstream is(filename, ifstream::in | ifstream::binary);
    if (is.is_open() == false)  {
        cerr << "file not open" << endl;
        return vector<unsigned char>();
    }
    is.seekg(0, ios::end);
    int size = is.tellg();
    string readStr;
    readStr.resize(size);
    is.seekg(0, ios::beg);
    is.read(&readStr[0], size);
    string toBase64 = EncodeBase64(readStr);
    return obtainHash(toBase64);
}
