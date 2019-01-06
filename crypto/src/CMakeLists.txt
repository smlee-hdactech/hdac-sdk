cmake_minimum_required(VERSION 3.1)

set(prj_base crypto)

configure_file(hs_${prj_base}.pc.in hs_${prj_base}.pc @ONLY)

set(src
    hmac_sha256.cpp
    hmac_sha512.cpp
    sha256.cpp
    sha512.cpp
    sha1.cpp
    ripemd160.cpp
    crypto.cpp
    )

add_library(${prj_base}_dynamic SHARED ${src})
add_library(${prj_base}_static STATIC ${src})

#link_directories(/usr/local/lib)

target_link_libraries(${prj_base}_dynamic
    json_spirit
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )
target_link_libraries(${prj_base}_static
    json_sprit
    ${SECP256K1_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::thread
    Boost::system
    )

target_include_directories(${prj_base}_dynamic
    PRIVATE
    ../include/${prj_base}
    )

target_include_directories(${prj_base}_static
    PRIVATE
    ../include/${prj_base}
    )

set_target_properties(${prj_base}_dynamic PROPERTIES OUTPUT_NAME "hdac_${prj_base}")
set_target_properties(${prj_base}_dynamic PROPERTIES VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} SOVERSION ${PROJECT_VERSION_MAJOR})

set_target_properties(${prj_base}_static PROPERTIES OUTPUT_NAME "hdac_${prj_base}")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install(TARGETS ${prj_base}_dynamic ${prj_base}_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

file(GLOB headers ../include/${prj_base}/*.h )
message(${headers})
#install(FILES ${headers} DESTINATION include/hdac/${prj_base})
install(FILES ${CMAKE_BINARY_DIR}/${prj_base}/src/hs_${prj_base}.pc DESTINATION lib/pkgconfig)
