#include "hashes.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <crypto/sha256.h>
#include <sstream>
#include <algorithm>
//#include <strcodeclib.h>
#include <utils/base64.h>

using namespace std;

void displayHashValue(unsigned char hash[CSHA256::OUTPUT_SIZE])
{
    for(size_t i = 0; i < CSHA256::OUTPUT_SIZE; i++) {
        cout << setw(2) << setfill('0') << hex << uppercase << (int)hash[i];
    }
    cout << endl;
}

void compareHashValue(unsigned char hash[CSHA256::OUTPUT_SIZE], const string& correct)
{
    ostringstream oStr;
    for(size_t i = 0; i < CSHA256::OUTPUT_SIZE; i++) {
        oStr << setw(2) << setfill('0') << hex << uppercase << (int)hash[i];
    }

    string upCorrect = correct;
    transform(upCorrect.begin(), upCorrect.end(), upCorrect.begin(), ::toupper);

    if (oStr.str() == upCorrect) {
        cout << "correct" << endl;
        return;
    }
    cout << "mismatch" << endl;
}

void obtainHash(const string& input, function<void(unsigned char hash[CSHA256::OUTPUT_SIZE])> callback)
{
    CSHA256 sha;
    unsigned char hash[CSHA256::OUTPUT_SIZE];
    sha.Write(reinterpret_cast<const unsigned char*>(input.c_str()), input.length()).Finalize(hash);
    callback(hash);
}

//void testCalcSHA256()
//{
//    using namespace std::placeholders;
//    //obtainHash("This is test message", displayHashValue);
//    // data from https://passwordsgenerator.net/sha256-hash-generator/
//    auto compareHash = bind(compareHashValue, _1, "DDDBDC2845C9D80DC288710D9B2CF2D6C4F613D0DC4C048A9EA0E8674C2C5E73");
//    obtainHash("This is test message", compareHash);
////    auto compareHash1 = bind(compareHashValue, _1, "CDDBDC2845C9D80DC288710D9B2CF2D6C4F613D0DC4C048A9EA0E8674C2C5E73");
////    obtainHash("This is test message", compareHash1);
//}

void hashFromFile(const string& filename)
{
    ifstream is(filename, ifstream::in | ifstream::binary);
    if (is.is_open() == false)  {
        cerr << "file not open" << endl;
        return;
    }
    is.seekg(0, ios::end);
    int size = is.tellg();
    string readStr;
    readStr.resize(size);
    is.seekg(0, ios::beg);
    is.read(&readStr[0], size);
    string toBase64 = EncodeBase64(readStr);
    obtainHash(toBase64, displayHashValue);
}
