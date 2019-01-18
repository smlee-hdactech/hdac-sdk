#ifndef UTILPARSE_H
#define UTILPARSE_H

struct mc_Script;
struct mc_Buffer;
class CScript;

bool FindFollowOnsInScript(const CScript& script1,mc_Buffer *amounts,mc_Script *lpScript);

#endif // UTILPARSE_H
