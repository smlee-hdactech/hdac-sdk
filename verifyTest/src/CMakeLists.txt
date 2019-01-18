cmake_minimum_required(VERSION 3.1)

add_executable(hdac_vTest main.cpp)

file(COPY params.dat
    DESTINATION ${PROJECT_BINARY_DIR}/bin)

target_include_directories(hdac_vTest 
    PRIVATE
    ../include/${prj_base}
    ${CMAKE_SOURCE_DIR}/structs/include
    ${CMAKE_SOURCE_DIR}/utils/include
    ${CMAKE_SOURCE_DIR}/crypto/include
    ${CMAKE_SOURCE_DIR}/rpc/include
    ${CMAKE_SOURCE_DIR}/keys/include
    ${CMAKE_SOURCE_DIR}/script/include
    ${CMAKE_SOURCE_DIR}/helpers/include
    ${JSONSPIRIT_INCLUDE_DIRS}
    )

if(UNIX)
target_link_libraries(hdac_vTest
    PUBLIC
    helpers_dynamic
    keys_dynamic
    script_dynamic
    structs_dynamic
    rpc_dynamic
    utils_dynamic
    crypto_dynamic
    ${SECP256K1_LIBRARIES}    
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )
else()
target_link_libraries(hdac_vTest
    PUBLIC
    helpers_static
    keys_static
    script_static
    structs_static
    rpc_static
    utils_static
    crypto_static
    ${SECP256K1_LIBRARIES}    
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
