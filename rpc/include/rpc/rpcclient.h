#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <string>
#include <json_spirit/json_spirit.h>

class RpcClient
{
public:
    RpcClient(const std::string& serverIp, int port, const std::string& user, const std::string& password, bool useSsl = false, const std::string& requestOut = "strerr") :
        _server(serverIp), _port(std::to_string(port)), _rpcuser(user), _rpcpassword(password), _fUseSSL(useSsl), _requestout(requestOut)
    {
    }

    RpcClient(int port, const std::string& user, const std::string& password, bool useSsl = false, const std::string& requestOut = "strerr") :
        RpcClient("localhost", port, user, password, useSsl, requestOut)
    {
    }

public:
    json_spirit::Object CallRPC(const std::string& strMethod,
                                const json_spirit::Array& params = json_spirit::Array());

private:
    std::string _port;
    std::string _rpcuser;
    std::string _rpcpassword;

    std::string _server; //"13.125.145.98"
    bool _fUseSSL;   // default : not use ssl
    std::string _requestout;
};

#endif // RPCCLIENT_H
