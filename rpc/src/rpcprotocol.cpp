#include "rpcprotocol.h"
#include <json_spirit/json_spirit_writer_template.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace json_spirit;

//! Number of bytes to allocate and read at most at once in post data
const size_t POST_READ_SIZE = 256 * 1024;

/**
 * HTTP protocol
 *
 * This ain't Apache.  We're just using HTTP header for the length field
 * and to be compatible with other JSON-RPC implementations.
 */
//#define PAIRTYPE(t1, t2)    std::pair<t1, t2>
string HTTPPost(const string& strMsg, const map<string,string>& mapRequestHeaders)
{
    const string clientVer = "1.0.0"; // TODO : to parameter
    ostringstream s;
    s << "POST / HTTP/1.1\r\n"
      << "User-Agent: hdac-json-rpc/" << clientVer << "\r\n"
      << "Host: 127.0.0.1\r\n"
      << "Content-Type: application/json\r\n"
      << "Content-Length: " << strMsg.size() << "\r\n"
      << "Connection: close\r\n"
      << "Accept: application/json\r\n";
    //BOOST_FOREACH(const PAIRTYPE(string, string)& item, mapRequestHeaders)
    for(const pair<string,string>& item : mapRequestHeaders)
        s << item.first << ": " << item.second << "\r\n";
    s << "\r\n" << strMsg;

    return s.str();
}

int ReadHTTPHeaders(std::basic_istream<char>& stream, map<string, string>& mapHeadersRet)
{
    int nLen = 0;
    while (true)
    {
        string str;
        std::getline(stream, str);
        if (str.empty() || str == "\r")
            break;
        string::size_type nColon = str.find(":");
        if (nColon != string::npos)
        {
            string strHeader = str.substr(0, nColon);
            boost::trim(strHeader);
            boost::to_lower(strHeader);
            string strValue = str.substr(nColon+1);
            boost::trim(strValue);
            mapHeadersRet[strHeader] = strValue;
            if (strHeader == "content-length")
                nLen = atoi(strValue.c_str());
        }
    }
    return nLen;
}

int ReadHTTPMessage(std::basic_istream<char>& stream, map<string, string>& mapHeadersRet, string& strMessageRet,
                    int nProto, size_t max_size)
{
    mapHeadersRet.clear();
    strMessageRet = "";

    // Read header
    int nLen = ReadHTTPHeaders(stream, mapHeadersRet);
    if (nLen < 0 || (size_t)nLen > max_size)
        return HTTP_INTERNAL_SERVER_ERROR;

    // Read message
    if (nLen > 0)
    {
        vector<char> vch;
        size_t ptr = 0;
        while (ptr < (size_t)nLen)
        {
            size_t bytes_to_read = std::min((size_t)nLen - ptr, POST_READ_SIZE);
            vch.resize(ptr + bytes_to_read);
            stream.read(&vch[ptr], bytes_to_read);
            if (!stream) // Connection lost while reading
                return HTTP_INTERNAL_SERVER_ERROR;
            ptr += bytes_to_read;
        }
        strMessageRet = string(vch.begin(), vch.end());
    }

    string sConHdr = mapHeadersRet["connection"];

    if ((sConHdr != "close") && (sConHdr != "keep-alive"))
    {
        if (nProto >= 1)
            mapHeadersRet["connection"] = "keep-alive";
        else
            mapHeadersRet["connection"] = "close";
    }

    return HTTP_OK;
}

/**
 * JSON-RPC protocol.  Bitcoin speaks version 1.0 for maximum compatibility,
 * but uses JSON-RPC 1.1/2.0 standards for parts of the 1.0 standard that were
 * unspecified (HTTP errors and contents of 'error').
 *
 * 1.0 spec: http://json-rpc.org/wiki/specification
 * 1.2 spec: http://jsonrpc.org/historical/json-rpc-over-http.html
 * http://www.codeproject.com/KB/recipes/JSON_Spirit.aspx
 */

string JSONRPCRequest(const string& strMethod, const Array& params, const Value& id)
{
    string chainName = "kcc"; // TODO : to parameter
    Object request;
    request.push_back(Pair("method", strMethod));
    request.push_back(Pair("params", params));
    request.push_back(Pair("id", id));
    request.push_back(Pair("chain_name", chainName));
    return write_string(Value(request), false) + "\n";
}

int ReadHTTPStatus(std::basic_istream<char>& stream, int &proto)
{
    string str;
    getline(stream, str);
    vector<string> vWords;
    boost::split(vWords, str, boost::is_any_of(" "));
    if (vWords.size() < 2)
        return HTTP_INTERNAL_SERVER_ERROR;
    proto = 0;
    const char *ver = strstr(str.c_str(), "HTTP/1.");
    if (ver != NULL)
        proto = atoi(ver+7);    // TODO : check value
    return atoi(vWords[1].c_str());
}
