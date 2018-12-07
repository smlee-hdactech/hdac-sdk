#include "rpcprotocol.h"

#include <json_spirit/json_spirit_reader_template.h>
#include <json_spirit/json_spirit_writer_template.h>
#include <utils/tinyformat.h>

using namespace std;
using namespace json_spirit;


int error(const Object& reply, string &strResult) {
    int nRet = 0;
    const Value& error  = find_value(reply, "error");

    if (error.type() == null_type) {
        return nRet;
    }

    const int code = find_value(error.get_obj(), "code").get_int();
    // TODO : how to implement
/*    if (fWait && code == RPC_IN_WARMUP)
        throw CConnectionFailed("server in warmup"); */
    //string strPrint;
    strResult = "error: " + write_string(error, false);
    nRet = abs(code);

    if (error.type() == obj_type)
    {
        Value errCode = find_value(error.get_obj(), "code");
        Value errMsg  = find_value(error.get_obj(), "message");
        strResult = (errCode.type() == null_type) ? "" : "error code: "+strprintf("%s",errCode.get_int())+"\n";

        if (errMsg.type() == str_type)
            strResult += "error message:\n"+errMsg.get_str();
    }
    fprintf(stderr, "%s\n", strResult.c_str());
    return nRet;
}

int result(const Object& reply, string &strResult)
{
    int nRet = ::error(reply, strResult);
    if (nRet != 0) {
        return nRet;
    }
    const Value& result = find_value(reply, "result");

    // Result
    if (result.type() == null_type)
        strResult = "";
    else if (result.type() == str_type)
        strResult = result.get_str();
    else
        strResult = write_string(result, true);

    //fprintf((nRet == 0 ? stdout : stderr), "%s\n", strResult.c_str());
    return nRet;
}




