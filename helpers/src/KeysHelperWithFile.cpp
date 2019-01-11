#include "KeysHelperWithFile.h"
#include <iostream>
#include <utils/utilstrencodings.h>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

char *string_trim_left(char *str)
{
    int i = 0;
    int j = 0;
    int len = 0;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (!IS_SPACE(str[i])) {
            for (j = i; j < len; j++) {
                str[j - i] = str[j];
            }
            str[len - i] = '\0';
            break;
        }
    }

    return str;
}

char *string_trim_right(char *str)
{
    int i = 0;
    int len = 0;

    len = strlen(str);

    for (i = len - 1; i >= 0; i--) {
        if (!IS_SPACE(str[i])) {
            break;
        } else {
            str[i] = '\0';
        }
    }

    return str;
}

char *string_trim(char *str)
{
    string_trim_left(str);
    string_trim_right(str);
    return str;
}

char *file_param_value_get(char *text, const char *key)
{
	char *front = NULL;
	char *back = NULL;
	int flen = 0;
	int blen = 0;

	char *token = NULL;

	front = strstr(text, key);
	if (front == NULL) {
		return NULL;
	}

	front = strchr(front, '=');
	if (front == NULL) {
		return NULL;
	}


	back = strchr(front, '#');
	if (back == NULL) {
		return NULL;
	}

	flen = strlen(front);
	blen = strlen(back);

	token = (char*)malloc(sizeof(char)*(flen - blen));
	if (token == NULL) {
		return NULL;
	}
	memset(token, 0x00, sizeof(char)*(flen - blen));
	snprintf(token, sizeof(char)*(flen - blen), "%s", front + 1);

	token = string_trim(token);

	return token;
}

map<string, string> mapFromFileReadMulti(const string& Path, 
						const vector<string>& keys)
{
	map<string, string> resultItems;
	char *temp;

	ifstream fop (Path.c_str(), ifstream::binary);
	if (!fop) {
		// error
		return resultItems;
	}	

	// get length of file:
	struct stat finfo;
	if (stat(Path.c_str(), &finfo) < 0) {
		// error
		return resultItems;
	}
	int length = finfo.st_size;

	char *buffer = new char [length];

	// read data as a block:
	fop.read (buffer,length);
	if (!fop) {
		fop.close();
	}

	fop.close();

	for (const string& key : keys) {
		temp = file_param_value_get(buffer, key.c_str());
		resultItems[key] = temp;
		free(temp);
	}

	delete[] buffer;

	return resultItems;
}

std::string trim(std::string& s,const std::string& drop = TRIM_SPACE)
{
	std::string r=s.erase(s.find_last_not_of(drop)+1);
	return r.erase(0,r.find_first_not_of(drop));
}
std::string rtrim(std::string s,const std::string& drop = TRIM_SPACE)
{
	return s.erase(s.find_last_not_of(drop)+1);
}
std::string ltrim(std::string s,const std::string& drop = TRIM_SPACE)
{
	return s.erase(0,s.find_first_not_of(drop));
}

map<string, string> mapFromFileReadAll(const string& Path) 
{
	map<string, string> resultItems;
	string line;

	ifstream openFile(Path.data());
	if(!openFile.is_open()){
		cout << "[" << Path << "] file open error" << endl;
		return resultItems;
	}

	while(getline(openFile, line)){
		if (line[0] == '#' || IS_SPACE(line[0]) ||
			line.size() == 0) {
			continue;
		}
		
		string key;
		string value;
		string comment;

		int first_idx;
		int second_idx;
		int value_len;

		first_idx = line.find("=");
		second_idx = line.find("#");

		key = line.substr(0, first_idx);
		comment = line.substr(second_idx);

		value_len = line.length() - key.length() - comment.length() - 1;

		value = line.substr(first_idx + 1, value_len);
		
		key = trim(key);
		value = trim(value);

		resultItems[key] = value;
	}
	openFile.close();

	return resultItems;
}

KeysHelperWithFileMulti::KeysHelperWithFileMulti(const string Path) {

	std::vector<string> keys{
		"address-pubkeyhash-version",
			"address-scripthash-version",
			"address-checksum-value",
			"private-key-version"
	};

	_resultMap = mapFromFileReadMulti(Path, keys);
	_addrHelper.reset(new WalletAddrHelper(_resultMap));
	_privHelper.reset(new PrivateKeyHelper(_resultMap));
}

const IWalletAddrHelper &KeysHelperWithFileMulti::addrHelper() const {
	return *_addrHelper;
}

const IPrivateKeyHelper &KeysHelperWithFileMulti::privHelper() const {
	return *_privHelper;
}

const std::vector<unsigned char> KeysHelperWithFileMulti::WalletAddrHelper::pubkeyAddrPrefix() const  {
	return ParseHex(_resultMap.at("address-pubkeyhash-version"));
}

const std::vector<unsigned char> KeysHelperWithFileMulti::WalletAddrHelper::scriptAddrPrefix() const  {
	return ParseHex(_resultMap.at("address-scripthash-version"));
}

int32_t KeysHelperWithFileMulti::WalletAddrHelper::addrChecksumValue() const  {
	return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
}

const std::vector<unsigned char> KeysHelperWithFileMulti::PrivateKeyHelper::privkeyPrefix() const  {
	return ParseHex(_resultMap.at("private-key-version"));
}

int32_t KeysHelperWithFileMulti::PrivateKeyHelper::addrChecksumValue() const  {
	return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
}

KeysHelperWithFileAll::KeysHelperWithFileAll(const string Path) {

	_resultMap = mapFromFileReadAll(Path);
	_addrHelper.reset(new WalletAddrHelper(_resultMap));
	_privHelper.reset(new PrivateKeyHelper(_resultMap));
}

const IWalletAddrHelper &KeysHelperWithFileAll::addrHelper() const {
	return *_addrHelper;
}

const IPrivateKeyHelper &KeysHelperWithFileAll::privHelper() const {
	return *_privHelper;
}

const std::vector<unsigned char> KeysHelperWithFileAll::WalletAddrHelper::pubkeyAddrPrefix() const  {
	return ParseHex(_resultMap.at("address-pubkeyhash-version"));
}

const std::vector<unsigned char> KeysHelperWithFileAll::WalletAddrHelper::scriptAddrPrefix() const  {
	return ParseHex(_resultMap.at("address-scripthash-version"));
}

int32_t KeysHelperWithFileAll::WalletAddrHelper::addrChecksumValue() const  {
	return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
}

const std::vector<unsigned char> KeysHelperWithFileAll::PrivateKeyHelper::privkeyPrefix() const  {
	return ParseHex(_resultMap.at("private-key-version"));
}

int32_t KeysHelperWithFileAll::PrivateKeyHelper::addrChecksumValue() const  {
	return parseHexToInt32Le(_resultMap.at("address-checksum-value"));
}

