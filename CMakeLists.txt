cmake_minimum_required(VERSION 3.1)

project(hdac-sdk)

set (CMAKE_CXX_STANDARD 11)
if(UNIX)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "hdac-sdk to develop solutions using hdac")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 2)
set(CPACK_PACKAGE_VERSION_PATCH 4)
#set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

include(CPack)
include(FindBoost)
include(FindOpenSSL)
if(UNIX)
include(FindPkgConfig)
endif()

find_package(Boost 1.55.0 REQUIRED system thread filesystem date_time)
find_package(OpenSSL REQUIRED)
if(UNIX)
pkg_check_modules (SECP256K1 REQUIRED libsecp256k1)
else()
find_path(LIBSECP256K1_INCLUDE_DIR secp256k1.h
          HINTS D:/job/window-porting/vcpkg/installed/x86-windows/include)
#message(${LIBSECP256K1_INCLUDE_DIR})
find_library(LIBSECP256K1_LIBRARY NAMES secp256k1.lib
             HINTS D:/job/window-porting/vcpkg/installed/x86-windows/lib )
#message(${LIBSECP256K1_LIBRARY})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSecp256k1 DEFAULT_MSG
                                  LIBSECP256K1_LIBRARY LIBSECP256K1_INCLUDE_DIR)
mark_as_advanced(LIBSECP256K1_INCLUDE_DIR LIBSECP256K1_LIBRARY )
set(SECP256K1_LIBRARIES ${LIBSECP256K1_LIBRARY} )
set(SECP256K1_INCLUDE_DIRS ${LIBSECP256K1_INCLUDE_DIR} )

find_path(LIBJSON_SPIRIT_INCLUDE_DIR json_spirit/json_spirit.h
          HINTS "C:/Program Files (x86)/json_spirit 4.0.8/include")
#message(${LIBJSON_SPIRIT_INCLUDE_DIR})
find_library(LIBJSON_SPIRIT_LIBRARY NAMES json_spirit.lib
             HINTS "C:/Program Files (x86)/json_spirit 4.0.8/lib" )
#message(${LIBJSON_SPIRIT_LIBRARY})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibJsonsprit DEFAULT_MSG
                                  LIBJSON_SPIRIT_LIBRARY LIBJSON_SPIRIT_INCLUDE_DIR)
mark_as_advanced(LIBJSON_SPIRIT_INCLUDE_DIR LIBJSON_SPIRIT_LIBRARY )
set(JSONSPIRIT_LIBRARIES ${LIBJSON_SPIRIT_LIBRARY} )
set(JSONSPIRIT_INCLUDE_DIRS ${LIBJSON_SPIRIT_INCLUDE_DIR} )

endif()

add_subdirectory(crypto)
add_subdirectory(utils)
add_subdirectory(structs)
add_subdirectory(script)
add_subdirectory(entities)
add_subdirectory(primitives)
add_subdirectory(keys)
add_subdirectory(rpc)
add_subdirectory(helpers)
add_subdirectory(moduleTest)
add_subdirectory(verifyTest)

