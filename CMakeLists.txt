cmake_minimum_required(VERSION 3.1)

project(hdac-sdk)

set (CMAKE_CXX_STANDARD 11)

add_subdirectory(crypto)
add_subdirectory(utils)
add_subdirectory(structs)
add_subdirectory(script)
add_subdirectory(keys)
add_subdirectory(rpc)
add_subdirectory(entities)
add_subdirectory(primitives)
#add_subdirectory(protocol)
add_subdirectory(moduleTest)

