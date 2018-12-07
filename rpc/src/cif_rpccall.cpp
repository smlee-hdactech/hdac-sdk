#include "cif_rpccall.h"
#include "rpccaller.h"
#include "rpcclient.h"

using namespace json_spirit;
using namespace std;

void getinfo()
{
    const bool fWait = false; // TODO : to parameter

    RpcClient client{"13.125.145.98", 4260, "hdacrpc", "1234"};
    const Object reply = client.CallRPC("getinfo", Array());

    string resultStr;
    int nRet = result(reply, resultStr);
    cout << resultStr << endl;
}
