cmake_minimum_required(VERSION 3.1)

project(HDAC_PRIMITIVIES_EXAMPLE)

set(targetName primitivesExample)
add_executable(${targetName} main.cpp)

if(UNIX)
target_link_libraries(${targetName}
    PUBLIC
    primitives_dynamic
    script_dynamic
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )
else()
target_link_libraries(${targetName}
    PUBLIC
    primitives_static
    script_static
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )
endif()    

target_include_directories(${targetName}
	PRIVATE
    ${CMAKE_SOURCE_DIR}/script/include
    ${CMAKE_SOURCE_DIR}/primitives/include
    ${JSONSPIRIT_INCLUDE_DIRS}
    )

