cmake_minimum_required(VERSION 3.1)

set(prj_base utils)

configure_file(hs_${prj_base}.pc.in hs_${prj_base}.pc @ONLY)

set(src
    base64.cpp
    utilstrencodings.cpp
    allocators.cpp
    random.cpp
    utiltime.cpp
    utility.cpp
    tools.cpp
    )

add_library(${prj_base}_dynamic SHARED ${src})
add_library(${prj_base}_static STATIC ${src})

target_link_libraries(${prj_base}_dynamic crypto_dynamic)
target_link_libraries(${prj_base}_static crypto_static)

target_include_directories(${prj_base}_dynamic 
    PRIVATE 
    ../include/${prj_base}
    ${CMAKE_SOURCE_DIR}/crypto/include)

target_include_directories(${prj_base}_static 
    PRIVATE 
    ../include/${prj_base}
    ${CMAKE_SOURCE_DIR}/crypto/include)

set_target_properties(${prj_base}_dynamic PROPERTIES OUTPUT_NAME "hdac_${prj_base}")
set_target_properties(${prj_base}_dynamic PROPERTIES VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} SOVERSION ${PROJECT_VERSION_MAJOR})

set_target_properties(${prj_base}_static PROPERTIES OUTPUT_NAME "hdac_${prj_base}")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install(TARGETS ${prj_base}_dynamic ${prj_base}_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

file(GLOB headers ../include/${prj_base}/*.h )
set(export_headers ../include/${prj_base}/utilstrencodings.h)
#install(FILES ${headers} DESTINATION include/hdac/${prj_base})
install(FILES ${export_headers} DESTINATION include/hdac/${prj_base})
install(FILES ${CMAKE_BINARY_DIR}/${prj_base}/src/hs_${prj_base}.pc DESTINATION lib/pkgconfig)
