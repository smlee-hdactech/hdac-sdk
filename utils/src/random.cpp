#include "random.h"

#include <iostream>
#include <cassert>

#include <crypto/sha512.h>

#include "utiltime.h"

#include <cstring>
#include <cstdlib>

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#else
#undef FEATURE_HDAC_QUANTUM_RANDOM_NUMBER	// WIN32 not support
#endif

#include <openssl/crypto.h>
#include <openssl/rand.h>

#include "util.h"

static void RandFailure()
{
    LogPrintf("Failed to read randomness, aborting\n");
    //std::cerr << "Failed to read randomness, aborting\n";
    abort();
}

static inline int64_t GetPerformanceCounter()
{
    int64_t nCounter = 0;
#ifdef WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&nCounter);
#else
    timeval t;
    gettimeofday(&t, NULL);
    nCounter = (int64_t)(t.tv_sec * 1000000 + t.tv_usec);
#endif
    return nCounter;
}

void RandAddSeed()
{
    // Seed with CPU performance counter
    int64_t nCounter = GetPerformanceCounter();
    RAND_add(&nCounter, sizeof(nCounter), 1.5);
    OPENSSL_cleanse((void*)&nCounter, sizeof(nCounter));
}

static void RandAddSeedPerfmon()
{
    RandAddSeed();

    // This can take up to 2 seconds, so only do it every 10 minutes
    static int64_t nLastPerfmon;
    if (GetTime() < nLastPerfmon + 10 * 60)
        return;
    nLastPerfmon = GetTime();

#ifdef WIN32
    // Don't need this on Linux, OpenSSL automatically uses /dev/urandom
    // Seed with the entire set of perfmon data
    std::vector<unsigned char> vData(250000, 0);
    long ret = 0;
    unsigned long nSize = 0;
    const size_t nMaxSize = 10000000; // Bail out at more than 10MB of performance data
    while (true) {
        nSize = vData.size();
        ret = RegQueryValueExA(HKEY_PERFORMANCE_DATA, "Global", NULL, NULL, begin_ptr(vData), &nSize);
        if (ret != ERROR_MORE_DATA || vData.size() >= nMaxSize)
            break;
        vData.resize(std::max((vData.size() * 3) / 2, nMaxSize)); // Grow size of buffer exponentially
    }
    RegCloseKey(HKEY_PERFORMANCE_DATA);
    if (ret == ERROR_SUCCESS) {
        RAND_add(begin_ptr(vData), nSize, nSize / 100.0);
        OPENSSL_cleanse(begin_ptr(vData), nSize);
        if(fDebug>1)LogPrint("rand", "%s: %lu bytes\n", __func__, nSize);
    } else {
        static bool warned = false; // Warn only once
        if (!warned) {
            LogPrintf("%s: Warning: RegQueryValueExA(HKEY_PERFORMANCE_DATA) failed with code %i\n", __func__, ret);
            warned = true;
        }
    }
#endif
}

/** Get 32 bytes of system entropy. */
static void GetOSRand(unsigned char *ent32)
{
#ifdef WIN32
    HCRYPTPROV hProvider;
    int ret = CryptAcquireContextW(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    if (!ret) {
        RandFailure();
    }
    ret = CryptGenRandom(hProvider, 32, ent32);
    if (!ret) {
        RandFailure();
    }
    CryptReleaseContext(hProvider, 0);
#else
    int f = open("/dev/urandom", O_RDONLY);
    if (f == -1) {
        RandFailure();
    }
    int have = 0;
    do {
        ssize_t n = read(f, ent32 + have, 32 - have);
        if (n <= 0 || n + have > 32) {
            RandFailure();
        }
        have += n;
    } while (have < 32);
    close(f);
#endif
}

void GetRandBytes_org(unsigned char* buf, int num)
{
    if (RAND_bytes(buf, num) != 1) {
        RandFailure();
//        if(fDebug>0)LogPrintf("%s: OpenSSL RAND_bytes() failed with error: %s\n", __func__, ERR_error_string(ERR_get_error(), NULL));
//        assert(false);
    }
}

void memory_cleanse(void *ptr, size_t len)
{
    OPENSSL_cleanse(ptr, len);
}

void GetStrongRandBytes_org(unsigned char* out, int num)
{
    assert(num <= 32);
    CSHA512 hasher;
    unsigned char buf[64];

    // First source: OpenSSL's RNG
    RandAddSeedPerfmon();
    GetRandBytes(buf, 32);
    hasher.Write(buf, 32);

    // Second source: OS RNG
    GetOSRand(buf);
    hasher.Write(buf, 32);

    // Produce output
    hasher.Finalize(buf);
    memcpy(out, buf, num);
    memory_cleanse(buf, 64);
}


#define QRNG_DEVICE0	"/dev/qrng_u3_0"
#define QRNG_DEVICE1	"/dev/qrng_u3_1"

static	int	_QRNG_fd = -1;


//
// EYL QRNG support function
// Two QRNG device support
//
void QRNG_RAND_bytes(unsigned char* out, int num)
{
    static time_t lasttime = 0;

    if (_QRNG_fd == -1)
        _QRNG_fd = open(QRNG_DEVICE0, O_RDONLY);
    if (_QRNG_fd == -1)
        _QRNG_fd = open(QRNG_DEVICE1, O_RDONLY);
    if (time(NULL) - lasttime > 30)
    {
        // TODO : change into log
        LogPrintf("%s: QRNG fd=%d\n", __func__, _QRNG_fd);
        //std::cout << __func__ << ": QRNG fd=" << _QRNG_fd << std::endl;
    }

    if (_QRNG_fd == -1)		// QRNG is not available
    {
        GetRandBytes_org(out, num);
        return;
    }
    if (time(NULL) - lasttime > 60) {
        // TODO : change into log
        //std::cout << __func__ << ": QRNG(Quantum Random Number Generator) endbaled and replaces RAND_bytes()." << std::endl;
        LogPrintf("%s: QRNG(Quantum Random Number Generator) endbaled and replaces RAND_bytes().\n", __func__);
    }
    lasttime = time(NULL);

    int nread = read(_QRNG_fd, out, num);
    // TODO : change into log
    //std::cout << __func__ << ": QRNG read bytes=" << nread << std::endl;
    LogPrintf("%s: QRNG read bytes=%d\n", __func__, nread);

    if (nread != num)	// read failed
    {
        _QRNG_fd = -1;
        GetRandBytes_org(out, num);
    }
}


void GetRandBytes(unsigned char* buf, int num)
{
    QRNG_RAND_bytes(buf, num);
}


//
// EYL QRNG support function
// Two QRNG device support
//
void QRNG_GetStrongRandBytes(unsigned char* out, int num)
{
    static time_t lasttime = 0;

    if (_QRNG_fd == -1)
        _QRNG_fd = open(QRNG_DEVICE0, O_RDONLY);
    if (_QRNG_fd == -1)
        _QRNG_fd = open(QRNG_DEVICE1, O_RDONLY);
    // TODO : change into log
    //std::cout << __func__ << ": QRNG fd=" << _QRNG_fd << std::endl;
    LogPrintf("%s: QRNG fd=%d\n", __func__, _QRNG_fd);

    if (_QRNG_fd == -1)		// QRNG is not available
    {
        GetStrongRandBytes_org(out, num);
        return;
    }
    if (time(NULL) - lasttime > 3600) {
        // TODO : change into log
        LogPrintf("%s: QRNG(Quantum Random Number Generator) endbaled and replaces GetStrongRandBytes().\n", __func__);
        //std::cout << __func__ << ": QRNG(Quantum Random Number Generator) endbaled and replaces GetStrongRandBytes()." << std::endl;
    }

    lasttime = time(NULL);

    int nread = read(_QRNG_fd, out, num);
    // TODO : change into log
    LogPrintf("%s: QRNG read=%d\n", __func__, nread);
    //std::cout << __func__ << ": QRNG read=" << nread << std::endl;

    if (nread != num)	// read failed
    {
        _QRNG_fd = -1;
        GetStrongRandBytes_org(out, num);
    }
}


void GetStrongRandBytes(unsigned char* out, int num)
{
    QRNG_GetStrongRandBytes(out, num);
}
