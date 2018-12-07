#ifndef RPCRESULT_H
#define RPCRESULT_H

#include <vector>
#include <string>
#include <map>

std::vector<std::string> findItemsFromRpcResult(const std::string& rpcResult, const std::vector<std::string>& keys);
std::map<std::string, std::string> mapFromRpcResult(const std::string& rpcResult, const std::vector<std::string>& keys);

#endif // RPCRESULT_H
