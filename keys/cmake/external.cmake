# Copyright (C) 2016-2017 Jonathan Müller <jonathanmueller.dev@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

#
# add hdac_utils
#
find_library(HDAC_STRUCTS_LIBRARY
    NAMES "hdac_strucs" "hdac_structs-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_STRUCTS_INCLUDE_DIR "structs/hashes.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

find_library(HDAC_SCRIPT_LIBRARY
    NAMES "hdac_script" "hdac_script-0.1.0"
    HINTS "/usr/lib" "/usr/local/lib")
find_path(HDAC_SCRIPT_INCLUDE_DIR "script/standard.h" "/usr/include/hdac/" "/usr/local/include/hdac/")

#if(NOT HDAC_STRUCTS_LIBRARY)
#    message("library not found")
#endif()

#if(NOT HDAC_STRUCTS_INCLUDE_DIR)
#    message("include not found")
#endif()


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

if((NOT HDAC_SCRIPT_LIBRARY) OR (NOT HDAC_SCRIPT_INCLUDE_DIR))
    message("Unable to find hdac_script, cloning...")
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
    add_library(hdac_script INTERFACE)
    message("inc dir: ${HDAC_SCRIPT_INCLUDE_DIR}")
    message("lib dir: ${HDAC_SCRIPT_LIBRARY}")
    target_include_directories(hdac_script INTERFACE ${HDAC_SCRIPT_INCLUDE_DIR})
    target_link_libraries(hdac_script INTERFACE ${HDAC_SCRIPT_LIBRARY})

    # install fake target
    install(TARGETS hdac_script EXPORT hdac_keysTargets DESTINATION ${lib_dir})
    message("check this")
endif()

include(FindPkgConfig)
pkg_check_modules (SECP256K1 REQUIRED libsecp256k1)
