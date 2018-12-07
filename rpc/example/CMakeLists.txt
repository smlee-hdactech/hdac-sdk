cmake_minimum_required(VERSION 3.1)

project(HDAC_RPC_EXAMPLE)

set(targetName rpcExample)
add_executable(${targetName} main.cpp)

find_package(Boost 1.55.0 REQUIRED system thread)
find_package(OpenSSL REQUIRED)

target_link_libraries(${targetName}
    PUBLIC
    rpc_dynamic
    utils_dynamic
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )

target_include_directories(${targetName}
	PRIVATE
	${CMAKE_SOURCE_DIR}/rpc/include)

