cmake_minimum_required(VERSION 3.1)

project(HDAC_RPC_EXAMPLE)

set(targetName rpcExample)
add_executable(${targetName} main.cpp)

if(UNIX)
target_link_libraries(${targetName}
    PUBLIC
    rpc_dynamic
    utils_dynamic
    ${JSONSPIRIT_LIBRARIES}
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}    
    Boost::thread
    Boost::system
    Boost::date_time
    )
else()
target_link_libraries(${targetName}
    PUBLIC
    rpc_static
    utils_static
    ${JSONSPIRIT_LIBRARIES}
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    Boost::date_time
    )
endif()    

target_include_directories(${targetName}
	PRIVATE    
	${CMAKE_SOURCE_DIR}/rpc/include
    ${JSONSPIRIT_INCLUDE_DIRS}
    )
    

