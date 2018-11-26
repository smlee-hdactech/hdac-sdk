add_executable(hdac_moduleTest main.cpp)

include(${CMAKE_SOURCE_DIR}/cmake/external.cmake)

target_include_directories(hdac_moduleTest
    PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include/> # for client in subdirectory
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for generated files in build mode
    $<INSTALL_INTERFACE:${${PROJECT_NAME}_INC_INSTALL_DIR}> # for client in install mode
    PRIVATE
    ${header_path}) # for source files

target_link_libraries(hdac_moduleTest
    PUBLIC
    hdac_keys
    hdac_rpc
    hdac_structs
    hdac_utils
    hdac_crypto
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    )
#target_link_libraries(hdac_moduleTest
#    PUBLIC
#    hdac_keys
#    hdac_structs
#    hdac_rpc
#    hdac_utils
#    hdac_crypto
#    ${Boost_LIBRARIES}
#    ${SECP256K1_LIBRARIES}
#    ${OPENSSL_LIBRARIES}
#    )
