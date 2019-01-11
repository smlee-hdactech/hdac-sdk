#include "util.h"

#include "utiltime.h"

#include <set>
#include <string>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

using namespace std;

bool fPrintToConsole = false;
bool fPrintToDebugLog = true;
bool fLogTimestamps = true;
bool fReopenDebugLog = false;
bool fLogTimeMillis = true;

/**
 * LogPrintf() has been broken a couple of times now
 * by well-meaning people adding mutexes in the most straightforward way.
 * It breaks because it may be called by global destructors during shutdown.
 * Since the order of destruction of static/global objects is undefined,
 * defining a mutex as a global object doesn't work (the mutex gets
 * destroyed, and then some later destructor calls OutputDebugStringF,
 * maybe indirectly, and you get a core dump at shutdown trying to lock
 * the mutex).
 */
static boost::once_flag debugPrintInitFlag = BOOST_ONCE_INIT;
/**
 * We use boost::call_once() to make sure these are initialized
 * in a thread-safe manner the first time called:
 */
static FILE* fileout = NULL;
static boost::mutex* mutexDebugLog = NULL;

static void DebugPrintInit()
{
    assert(fileout == NULL);
    assert(mutexDebugLog == NULL);

    // TODO : how to config the location for log file
    //boost::filesystem::path pathDebug = GetDataDir() / "debug.log";
    boost::filesystem::path pathDebug = "debug.log";
    fileout = fopen(pathDebug.string().c_str(), "a");
    if (fileout) setbuf(fileout, NULL); // unbuffered

    mutexDebugLog = new boost::mutex();
}


void DebugPrintClose()
{
    if(fileout)
    {
        fclose(fileout);
        fileout=NULL;
    }
}

bool LogAcceptCategory(const char* category)
{
    if (category != NULL)
    {
        // TODO : how to handle fDebug
#if 0
        if (!fDebug)
            return false;
#endif

        // Give each thread quick access to -debug settings.
        // This helps prevent issues debugging global destructors,
        // where mapMultiArgs might be deleted before another
        // global destructor calls LogPrint()
        static boost::thread_specific_ptr<set<string> > ptrCategory;
        if (ptrCategory.get() == NULL)
        {
            // TODO : how to handle categories
            //const vector<string>& categories = mapMultiArgs["-debug"];
            const vector<string>& categories = vector<string>();
            ptrCategory.reset(new set<string>(categories.begin(), categories.end()));
            // thread_specific_ptr automatically deletes the set when the thread ends.
        }

        const set<string>& setCategories = *ptrCategory.get();

        // if not debugging everything and not debugging specific category, LogPrint does nothing.
        if (setCategories.count(string("")) == 0 &&
            setCategories.count(string(category)) == 0)
            return false;
    }
    return true;
}

int LogPrintStr(const std::string &str)
{
    int ret = 0; // Returns total number of characters written
    if (fPrintToConsole)
    {
        // print to console
        ret = fwrite(str.data(), 1, str.size(), stdout);
        fflush(stdout);
    }
    //else if (fPrintToDebugLog && AreBaseParamsConfigured())
    else if (fPrintToDebugLog)
    {
        static bool fStartedNewLine = true;
        boost::call_once(&DebugPrintInit, debugPrintInitFlag);

        if (fileout == NULL)
            return ret;

        boost::mutex::scoped_lock scoped_lock(*mutexDebugLog);

        // reopen the log file, if requested
        if (fReopenDebugLog) {
            fReopenDebugLog = false;
            // TODO : how to config the location for log file.
            //boost::filesystem::path pathDebug = GetDataDir() / "debug.log";
            boost::filesystem::path pathDebug = "debug.log";
            if (freopen(pathDebug.string().c_str(),"a",fileout) != NULL)
                setbuf(fileout, NULL); // unbuffered
        }

        // Debug print useful for profiling
        if (fLogTimestamps && fStartedNewLine)

            ret += fprintf(fileout, "%s", DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()).c_str());
        if (fLogTimestamps && fStartedNewLine)
        {
            if (fLogTimeMillis)
            {
                ret += fprintf(fileout, ".%03d ",(int)(GetTimeMillis()%1000));
            }
            else
            {
                ret += fprintf(fileout, " ");
            }
        }

        if (!str.empty() && str[str.size()-1] == '\n')
            fStartedNewLine = true;
        else
            fStartedNewLine = false;

        ret = fwrite(str.data(), 1, str.size(), fileout);
    }

    return ret;
}
