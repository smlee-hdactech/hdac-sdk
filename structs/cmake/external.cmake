# Copyright (C) 2016-2017 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

#
# add hdac_utils
#
find_library(HDAC_UTILS_LIBRARY
    NAMES "hdac_utils" "hdac_utils-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_UTILS_INCLUDE_DIR "utils/strcodeclib.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

find_library(HDAC_CRYPTO_LIBRARY
    NAMES "hdac_crypto" "hdac_crypto-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_CRYPTO_INCLUDE_DIR "crypto/crypto.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

#if(NOT HDAC_UTILS_LIBRARY)
#    message("library not found")
#endif()

#if(NOT HDAC_UTILS_INCLUDE_DIR)
#    message("include not found")
#endif()

if(NOT HDAC_CRYPTO_LIBRARY)
    message("crypto library not found")
endif()

if(NOT HDAC_CRYPTO_INCLUDE_DIR)
    message("crypto include not found")
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

