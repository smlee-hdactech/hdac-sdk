#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <string>
#include <json_spirit/json_spirit.h>

struct RpcAccess
{
    RpcAccess(const int port_, const std::string& user_, const std::string& password_, const std::string& serverIp_ = "localhost") :
        port(std::to_string(port_)), rpcuser(user_), rpcpassword(password_), server(serverIp_)  {}

    std::string port;
    std::string rpcuser;
    std::string rpcpassword;
    std::string server; //"13.125.145.98"
};

class RpcClient
{
public:
    RpcClient(const RpcAccess& access, const std::string &chainName = "hdac", bool useSsl = false, const std::string& requestOut = "strerr") :
        _accessInfo(access), _chainName(chainName), _fUseSSL(useSsl), _requestout(requestOut) {}

    RpcClient(const std::string& serverIp, int port, const std::string& user, const std::string& password, const std::string& chainName = "hdac",
              bool useSsl = false, const std::string& requestOut = "strerr") :
        RpcClient(RpcAccess(port, user, password, serverIp), chainName, useSsl, requestOut) {}

    RpcClient(int port, const std::string& user, const std::string& password, const std::string& chainName = "hdac",
              bool useSsl = false, const std::string& requestOut = "strerr") :
        RpcClient(RpcAccess(port, user, password), chainName, useSsl, requestOut)   {}

public:
    json_spirit::Object CallRPC(const std::string& strMethod,
                                const json_spirit::Array& params = json_spirit::Array()) const;

private:
    RpcAccess _accessInfo;

    std::string _chainName;

    bool _fUseSSL;   // default : not use ssl
    std::string _requestout;
};

#endif // RPCCLIENT_H
