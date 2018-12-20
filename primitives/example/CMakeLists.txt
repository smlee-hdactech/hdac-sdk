cmake_minimum_required(VERSION 3.1)

project(HDAC_KEYS_EXAMPLE)

set(targetName keysExample)
add_executable(${targetName} main.cpp)

find_package(Boost 1.55.0 REQUIRED system thread)
find_package(OpenSSL REQUIRED)
pkg_check_modules (SECP256K1 REQUIRED libsecp256k1)

target_link_libraries(${targetName}
    PUBLIC
    keys_dynamic
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )

target_include_directories(${targetName}
	PRIVATE
        ${CMAKE_SOURCE_DIR}/crypto/include
        ${CMAKE_SOURCE_DIR}/utils/include
        ${CMAKE_SOURCE_DIR}/structs/include
        ${CMAKE_SOURCE_DIR}/script/include
        ${CMAKE_SOURCE_DIR}/keys/include)

