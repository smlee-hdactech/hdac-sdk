#ifndef STRCODECLIB_GLOBAL_H
#define STRCODECLIB_GLOBAL_H

#if (defined _WIN32) || (defined _WIN64)
    #define Q_DECL_EXPORT __declspec(dllexport)
    #define Q_DECL_IMPORT __declspec(dllimport)
#else
    #define Q_DECL_EXPORT
    #define Q_DECL_IMPORT
#endif

#if defined(STRCODECLIB_LIBRARY)
#  define STRCODECLIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define STRCODECLIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // STRCODECLIB_GLOBAL_H
