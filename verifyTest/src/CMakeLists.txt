cmake_minimum_required(VERSION 3.1)

add_executable(hdac_vTest main.cpp)

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
    )

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

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
