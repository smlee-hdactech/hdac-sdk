#ifndef HASHES_GLOBAL_H
#define HASHES_GLOBAL_H

#if (defined _WIN32) || (defined _WIN64)
    #define Q_DECL_EXPORT __declspec(dllexport)
    #define Q_DECL_IMPORT __declspec(dllimport)
#else
    #define Q_DECL_EXPORT
    #define Q_DECL_IMPORT
#endif

#if defined(HASHES_LIBRARY)
#  define HASHESSHARED_EXPORT Q_DECL_EXPORT
#else
#  define HASHESSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // HASHES_GLOBAL_H
