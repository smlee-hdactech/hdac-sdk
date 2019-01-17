#include "rpcclient.h"
#include "rpcprotocol.h"
#include <utils/base64.h>
#include <utils/tinyformat.h>
#include <json_spirit/json_spirit_reader_template.h>

using namespace json_spirit;
using namespace std;
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

Object RpcClient::CallRPC(const string& strMethod, const Array& params) const
{
    asio::io_service io_service;
#ifdef WIN32
    ssl::context context(ssl::context::sslv23);
#else    
    ssl::context context(io_service, ssl::context::sslv23);
#endif    
    asio::ssl::stream<asio::ip::tcp::socket> sslStream(io_service, context);
    SSLIOStreamDevice<asio::ip::tcp> d(sslStream, _fUseSSL);
    iostreams::stream< SSLIOStreamDevice<asio::ip::tcp> > stream(d);

    const bool fConnected = d.connect(_accessInfo.server, _accessInfo.port);
    if (!fConnected)
        throw CConnectionFailed("couldn't connect to server");

    string strUserPass64 = EncodeBase64(_accessInfo.rpcuser + ":" + _accessInfo.rpcpassword);
    map<string, string> mapRequestHeaders;
    mapRequestHeaders["Authorization"] = string("Basic ") + strUserPass64;

    string strRequest = JSONRPCRequest(strMethod, params, 1, _chainName);
    string strPost = HTTPPost(strRequest, mapRequestHeaders);
    stream << strPost << std::flush;

    if(_requestout == "stdout")
    {
        fprintf(stdout, "%s\n", strRequest.c_str());
    }
    if(_requestout == "stderr")
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
