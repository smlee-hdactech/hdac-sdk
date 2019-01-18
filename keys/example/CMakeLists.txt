cmake_minimum_required(VERSION 3.1)

project(HDAC_KEYS_EXAMPLE)

set(targetName keysExample)
add_executable(${targetName} main.cpp)

if(UNIX)
target_link_libraries(${targetName}
    PUBLIC
    keys_dynamic
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )
else()
target_link_libraries(${targetName}
    PUBLIC
    keys_static
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )
endif()

target_include_directories(${targetName}
	PRIVATE
    ${CMAKE_SOURCE_DIR}/crypto/include
    ${CMAKE_SOURCE_DIR}/utils/include
    ${CMAKE_SOURCE_DIR}/structs/include
    ${CMAKE_SOURCE_DIR}/script/include
    ${CMAKE_SOURCE_DIR}/keys/include
    )

