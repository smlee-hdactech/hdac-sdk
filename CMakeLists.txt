cmake_minimum_required(VERSION 3.1)

project(hdac-sdk)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")

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

