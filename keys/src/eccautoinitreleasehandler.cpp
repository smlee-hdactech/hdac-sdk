#include "eccautoinitreleasehandler.h"
#include "key.h"
#include <iostream>
using namespace std;

EccAutoInitReleaseHandler::EccAutoInitReleaseHandler() {
    ECC_Start();
    verifyHandle.reset(new ECCVerifyHandle);
    if(!ECC_InitSanityCheck()) {
        cerr << "Elliptic curve cryptography sanity check failure. Aborting." << endl;
        //InitError("Elliptic curve cryptography sanity check failure. Aborting.");
        verifyHandle.reset();
        ECC_Stop();
        return;
    }
}

EccAutoInitReleaseHandler::~EccAutoInitReleaseHandler() {
    ECC_Stop();
}
