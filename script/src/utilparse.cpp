#include "utilparse.h"
#include <utils/utility.h>
#include "script.h"
#include "hdacscript.h"
#include <utils/define.h>

bool FindFollowOnsInScript(const CScript& script1,mc_Buffer *amounts,mc_Script *lpScript)
{
    int err;
    CScript::const_iterator pc1 = script1.begin();

    lpScript->Clear();
    lpScript->SetScript((unsigned char*)(&pc1[0]),(size_t)(script1.end()-pc1),MC_SCR_TYPE_SCRIPTPUBKEY);

    for (int e = 0; e < lpScript->GetNumElements(); e++)                        // Parsing asset quantities
    {
        lpScript->SetElement(e);
        err=lpScript->GetAssetQuantities(amounts,MC_SCR_ASSET_SCRIPT_TYPE_FOLLOWON);
        if((err != MC_ERR_NOERROR) && (err != MC_ERR_WRONG_SCRIPT))
        {
            return false;
        }
    }
    return true;
}
