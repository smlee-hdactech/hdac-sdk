#ifndef UTIL_H
#define UTIL_H

#include "tinyformat.h"

/** Return true if log accepts specified category */
bool LogAcceptCategory(const char* category);
/** Send a string to the log output */
int LogPrintStr(const std::string &str);

#define LogPrintf(...) LogPrint(NULL, __VA_ARGS__)

/**
 * When we switch to C++11, this can be switched to variadic templates instead
 * of this macro-based construction (see tinyformat.h).
 */
#define MAKE_ERROR_AND_LOG_FUNC(n)                                        \
    /**   Print to debug.log if -debug=category switch is given OR category is NULL. */ \
    template<TINYFORMAT_ARGTYPES(n)>                                          \
    static inline int LogPrint(const char* category, const char* format, TINYFORMAT_VARARGS(n))  \
    {                                                                         \
        if(!LogAcceptCategory(category)) return 0;                            \
        return LogPrintStr(tfm::format(format, TINYFORMAT_PASSARGS(n))); \
    }                                                                         \
    /**   Log error and return false */                                        \
    template<TINYFORMAT_ARGTYPES(n)>                                          \
    static inline bool error(const char* format, TINYFORMAT_VARARGS(n))                     \
    {                                                                         \
        LogPrintStr("ERROR: " + tfm::format(format, TINYFORMAT_PASSARGS(n)) + "\n"); \
        return false;                                                         \
    }										\
    /**   Log error status and return false */                                \
    template<TINYFORMAT_ARGTYPES(n)>							\
    static inline bool error_status(const char* format, TINYFORMAT_VARARGS(n))		\
    {											\
        LogPrintStr("Status: " + tfm::format(format, TINYFORMAT_PASSARGS(n)) + "\n");	\
        return false;									\
    }

TINYFORMAT_FOREACH_ARGNUM(MAKE_ERROR_AND_LOG_FUNC)

/**
 * Zero-arg versions of logging and error, these are not covered by
 * TINYFORMAT_FOREACH_ARGNUM
 */
static inline int LogPrint(const char* category, const char* format)
{
    if(!LogAcceptCategory(category)) return 0;
    return LogPrintStr(format);
}

#endif // UTIL_H
