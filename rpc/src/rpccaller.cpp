#include "rpcprotocol.h"
#include <utils/base64.h>
#include <utils/tinyformat.h>
#include <json_spirit/json_spirit_reader_template.h>
#include <json_spirit/json_spirit_writer_template.h>

using namespace std;
using namespace json_spirit;
using namespace boost;
using namespace boost::asio;

//
// Exception thrown on connection error.  This error is used to determine
// when to wait if -rpcwait is given.
//
class CConnectionFailed : public std::runtime_error
{
public:

    explicit inline CConnectionFailed(const std::string& msg) :
        std::runtime_error(msg)
    {}

};

Object CallRPC(const string& strMethod, const Array& params)
{
    // TODO : to parameter
    bool fUseSSL = false;   // default : not use ssl
    string server = "13.125.145.98";
    string port = "4260";
    string rpcuser = "hdacrpc";
    string rpcpassword = "1234";
    string requestout = "stderr";

    asio::io_service io_service;
    ssl::context context(io_service, ssl::context::sslv23);
    asio::ssl::stream<asio::ip::tcp::socket> sslStream(io_service, context);
    SSLIOStreamDevice<asio::ip::tcp> d(sslStream, fUseSSL);
    iostreams::stream< SSLIOStreamDevice<asio::ip::tcp> > stream(d);

    const bool fConnected = d.connect(server, port);
    if (!fConnected)
        throw CConnectionFailed("couldn't connect to server");

    string strUserPass64 = EncodeBase64(rpcuser + ":" + rpcpassword);
    map<string, string> mapRequestHeaders;
    mapRequestHeaders["Authorization"] = string("Basic ") + strUserPass64;

    string strRequest = JSONRPCRequest(strMethod, params, 1);
    string strPost = HTTPPost(strRequest, mapRequestHeaders);
    stream << strPost << std::flush;

    if(requestout == "stdout")
    {
        fprintf(stdout, "%s\n", strRequest.c_str());
    }
    if(requestout == "stderr")
    {
        fprintf(stderr, "%s\n", strRequest.c_str());
    }

    // Receive HTTP reply status
    int nProto = 0;
    int nStatus = ReadHTTPStatus(stream, nProto);

    // Receive HTTP reply message headers and body
    map<string, string> mapHeaders;
    string strReply;
    ReadHTTPMessage(stream, mapHeaders, strReply, nProto, std::numeric_limits<size_t>::max());

    if (nStatus == HTTP_UNAUTHORIZED)
        throw runtime_error("incorrect rpcuser or rpcpassword (authorization failed)");
    else if (nStatus >= 400 && nStatus != HTTP_BAD_REQUEST && nStatus != HTTP_NOT_FOUND && nStatus != HTTP_INTERNAL_SERVER_ERROR)
        throw runtime_error(strprintf("server returned HTTP error %d", nStatus));
    else if (strReply.empty())
        throw runtime_error("no response from server");

    // Parse reply
    Value valReply;
    if (!read_string(strReply, valReply))
        throw runtime_error("couldn't parse reply from server");
    const Object& reply = valReply.get_obj();
    if (reply.empty())
        throw runtime_error("expected reply to have result, error and id properties");

    return reply;
}

void ShowResultWithRPC(const string &method, const Array &params)
{
    const bool fWait = false; // TODO : to parameter
    const Object reply = CallRPC(method, params);

    // Parse reply
    const Value& result = find_value(reply, "result");
    const Value& error  = find_value(reply, "error");

    // TODO : display handling with error occurred
    string strPrint;
    int nRet = 0;
    if (error.type() != null_type) {
        // Error
        const int code = find_value(error.get_obj(), "code").get_int();
        if (fWait && code == RPC_IN_WARMUP)
            throw CConnectionFailed("server in warmup");
        strPrint = "error: " + write_string(error, false);
        nRet = abs(code);

        if (error.type() == obj_type)
        {
            Value errCode = find_value(error.get_obj(), "code");
            Value errMsg  = find_value(error.get_obj(), "message");
            strPrint = (errCode.type() == null_type) ? "" : "error code: "+strprintf("%s",errCode.get_int())+"\n";

            if (errMsg.type() == str_type)
                strPrint += "error message:\n"+errMsg.get_str();
        }

    } else {
        // Result
        if (result.type() == null_type)
            strPrint = "";
        else if (result.type() == str_type)
            strPrint = result.get_str();
        else
            strPrint = write_string(result, true);
    }

    if (strPrint != "") {
        fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
    }
}


