cmake_minimum_required(VERSION 3.1)

project(hdac-sdk)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "hdac-sdk to develop solutions using hdac")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 2)
set(CPACK_PACKAGE_VERSION_PATCH 0)
#set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

include(CPack)
include(FindBoost)
include(FindOpenSSL)
include(FindPkgConfig)

find_package(Boost 1.55.0 REQUIRED system thread)
find_package(OpenSSL REQUIRED)
pkg_check_modules (SECP256K1 REQUIRED libsecp256k1)

add_subdirectory(crypto)
add_subdirectory(utils)
add_subdirectory(structs)
add_subdirectory(script)
add_subdirectory(keys)
add_subdirectory(rpc)
add_subdirectory(entities)
add_subdirectory(primitives)
add_subdirectory(helpers)
add_subdirectory(moduleTest)
add_subdirectory(verifyTest)

