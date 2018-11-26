#include "cif_rpccall.h"
#include "rpccaller.h"

using namespace json_spirit;

void getinfo()
{
    ShowResultWithRPC("getinfo", Array());
}
