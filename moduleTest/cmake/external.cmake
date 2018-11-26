# Copyright (C) 2016-2017 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

#
# add hdac_stucts
#
find_library(HDAC_STRUCTS_LIBRARY
    NAMES "hdac_structs" "hdac_structs-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_STRUCTS_INCLUDE_DIR "structs/hashes.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

find_library(HDAC_UTILS_LIBRARY
    NAMES "hdac_utils" "hdac_utils-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_UTILS_INCLUDE_DIR "utils/strcodeclib.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

find_library(HDAC_CRYPTO_LIBRARY
    NAMES "hdac_crypto" "hdac_crypto-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_CRYPTO_INCLUDE_DIR "crypto/crypto.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

find_library(HDAC_RPC_LIBRARY
    NAMES "hdac_rpc" "hdac_rpc-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_RPC_INCLUDE_DIR "rpc/rpccaller.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

find_library(HDAC_KEYS_LIBRARY
    NAMES "hdac_keys" "hdac_keys-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_KEYS_INCLUDE_DIR "keys/keyslib.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

if(NOT HDAC_STRUCTS_LIBRARY)
    message("library not found")
endif()

if(NOT HDAC_STRUCTS_INCLUDE_DIR)
    message("include not found")
endif()


if((NOT HDAC_STRUCTS_LIBRARY) OR (NOT HDAC_STRUCTS_INCLUDE_DIR))
    message("Unable to find hdac_structs, cloning...")
    #execute_process(COMMAND git submodule update --init -- external/cmark
    #                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # add and exclude targets
    #add_subdirectory(external/cmark ${CMAKE_CURRENT_BINARY_DIR}/cmark EXCLUDE_FROM_ALL)
    #set_target_properties(api_test PROPERTIES EXCLUDE_FROM_ALL 1)

    # fixup target properties
    #target_include_directories(libcmark-gfm_static SYSTEM PUBLIC
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cmark/src>
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cmark/src>)
    #target_compile_definitions(libcmark-gfm_static PUBLIC CMARK_GFM_STATIC_DEFINE)

    # disable some warnings under MSVC, they're very noisy
    #if(MSVC)
    #    target_compile_options(libcmark-gfm_static PRIVATE /wd4204 /wd4267 /wd4204 /wd4221 /wd4244 /wd4232)
    #endif()
else()
    add_library(hdac_structs INTERFACE)
    message("inc dir: ${HDAC_STRUCTS_INCLUDE_DIR}")
    message("lib dir: ${HDAC_STRUCTS_LIBRARY}")
    target_include_directories(hdac_structs INTERFACE ${HDAC_STRUCTS_INCLUDE_DIR})
    target_link_libraries(hdac_structs INTERFACE ${HDAC_STRUCTS_LIBRARY})

    # install fake target
    install(TARGETS hdac_structs EXPORT hdac_keysTargets DESTINATION ${lib_dir})
    message("check this")
endif()

if((NOT HDAC_UTILS_LIBRARY) OR (NOT HDAC_UTILS_INCLUDE_DIR))
    message("Unable to find hdac_utils, cloning...")
    #execute_process(COMMAND git submodule update --init -- external/cmark
    #                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # add and exclude targets
    #add_subdirectory(external/cmark ${CMAKE_CURRENT_BINARY_DIR}/cmark EXCLUDE_FROM_ALL)
    #set_target_properties(api_test PROPERTIES EXCLUDE_FROM_ALL 1)

    # fixup target properties
    #target_include_directories(libcmark-gfm_static SYSTEM PUBLIC
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cmark/src>
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cmark/src>)
    #target_compile_definitions(libcmark-gfm_static PUBLIC CMARK_GFM_STATIC_DEFINE)

    # disable some warnings under MSVC, they're very noisy
    #if(MSVC)
    #    target_compile_options(libcmark-gfm_static PRIVATE /wd4204 /wd4267 /wd4204 /wd4221 /wd4244 /wd4232)
    #endif()
else()
    add_library(hdac_utils INTERFACE)
    message("inc dir: ${HDAC_UTILS_INCLUDE_DIR}")
    message("lib dir: ${HDAC_UTILS_LIBRARY}")
    target_include_directories(hdac_utils INTERFACE ${HDAC_UTILS_INCLUDE_DIR})
    target_link_libraries(hdac_utils INTERFACE ${HDAC_UTILS_LIBRARY})

    # install fake target
    install(TARGETS hdac_utils EXPORT hdac_structsTargets DESTINATION ${lib_dir})
    message("check this")
endif()

if((NOT HDAC_CRYPTO_LIBRARY) OR (NOT HDAC_CRYPTO_INCLUDE_DIR))
    message("Unable to find hdac_crypto, cloning...")
    #execute_process(COMMAND git submodule update --init -- external/cmark
    #                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # add and exclude targets
    #add_subdirectory(external/cmark ${CMAKE_CURRENT_BINARY_DIR}/cmark EXCLUDE_FROM_ALL)
    #set_target_properties(api_test PROPERTIES EXCLUDE_FROM_ALL 1)

    # fixup target properties
    #target_include_directories(libcmark-gfm_static SYSTEM PUBLIC
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cmark/src>
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cmark/src>)
    #target_compile_definitions(libcmark-gfm_static PUBLIC CMARK_GFM_STATIC_DEFINE)

    # disable some warnings under MSVC, they're very noisy
    #if(MSVC)
    #    target_compile_options(libcmark-gfm_static PRIVATE /wd4204 /wd4267 /wd4204 /wd4221 /wd4244 /wd4232)
    #endif()
else()
    add_library(hdac_crypto INTERFACE)
    message("inc dir: ${HDAC_CRYPTO_INCLUDE_DIR}")
    message("lib dir: ${HDAC_CRYPTO_LIBRARY}")
    target_include_directories(hdac_crypto INTERFACE ${HDAC_CRYPTO_INCLUDE_DIR})
    target_link_libraries(hdac_crypto INTERFACE ${HDAC_CRYPTO_LIBRARY})

    # install fake target
    install(TARGETS hdac_crypto EXPORT hdac_structsTargets DESTINATION ${lib_dir})
    message("check this")
endif()

if((NOT HDAC_RPC_LIBRARY) OR (NOT HDAC_RPC_INCLUDE_DIR))
    message("Unable to find hdac_rpc, cloning...")
    #execute_process(COMMAND git submodule update --init -- external/cmark
    #                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # add and exclude targets
    #add_subdirectory(external/cmark ${CMAKE_CURRENT_BINARY_DIR}/cmark EXCLUDE_FROM_ALL)
    #set_target_properties(api_test PROPERTIES EXCLUDE_FROM_ALL 1)

    # fixup target properties
    #target_include_directories(libcmark-gfm_static SYSTEM PUBLIC
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cmark/src>
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cmark/src>)
    #target_compile_definitions(libcmark-gfm_static PUBLIC CMARK_GFM_STATIC_DEFINE)

    # disable some warnings under MSVC, they're very noisy
    #if(MSVC)
    #    target_compile_options(libcmark-gfm_static PRIVATE /wd4204 /wd4267 /wd4204 /wd4221 /wd4244 /wd4232)
    #endif()
else()
    add_library(hdac_rpc INTERFACE)
    message("inc dir: ${HDAC_RPC_INCLUDE_DIR}")
    message("lib dir: ${HDAC_RPC_LIBRARY}")
    target_include_directories(hdac_rpc INTERFACE ${HDAC_RPC_INCLUDE_DIR})
    target_link_libraries(hdac_rpc INTERFACE ${HDAC_RPC_LIBRARY})

    # install fake target
    install(TARGETS hdac_rpc EXPORT hdac_structsTargets DESTINATION ${lib_dir})
    message("check this")
endif()

if((NOT HDAC_KEYS_LIBRARY) OR (NOT HDAC_KEYS_INCLUDE_DIR))
    message("Unable to find hdac_crypto, cloning...")
    #execute_process(COMMAND git submodule update --init -- external/cmark
    #                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # add and exclude targets
    #add_subdirectory(external/cmark ${CMAKE_CURRENT_BINARY_DIR}/cmark EXCLUDE_FROM_ALL)
    #set_target_properties(api_test PROPERTIES EXCLUDE_FROM_ALL 1)

    # fixup target properties
    #target_include_directories(libcmark-gfm_static SYSTEM PUBLIC
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cmark/src>
    #                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cmark/src>)
    #target_compile_definitions(libcmark-gfm_static PUBLIC CMARK_GFM_STATIC_DEFINE)

    # disable some warnings under MSVC, they're very noisy
    #if(MSVC)
    #    target_compile_options(libcmark-gfm_static PRIVATE /wd4204 /wd4267 /wd4204 /wd4221 /wd4244 /wd4232)
    #endif()
else()
    add_library(hdac_keys INTERFACE)
    message("inc dir: ${HDAC_KEYS_INCLUDE_DIR}")
    message("lib dir: ${HDAC_KEYS_LIBRARY}")
    target_include_directories(hdac_keys INTERFACE ${HDAC_KEYS_INCLUDE_DIR})
    target_link_libraries(hdac_keys INTERFACE ${HDAC_KEYS_LIBRARY})

    # install fake target
    install(TARGETS hdac_keys EXPORT hdac_structsTargets DESTINATION ${lib_dir})
    message("check this")
endif()

find_package (Boost 1.55.0 REQUIRED COMPONENTS system thread)

include(FindPkgConfig)
pkg_check_modules (SECP256K1 REQUIRED libsecp256k1)
pkg_check_modules (OPENSSL REQUIRED openssl)
