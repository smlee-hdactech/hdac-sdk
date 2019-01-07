cmake_minimum_required(VERSION 3.1)

set(prj_base helpers)

configure_file(hs_${prj_base}.pc.in hs_${prj_base}.pc @ONLY)

set(src
    KeysHelperWithRpc.cpp
    KeysHelperWithFile.cpp
    )

add_library(${prj_base}_dynamic SHARED ${src})
add_library(${prj_base}_static STATIC ${src})

target_link_libraries(${prj_base}_dynamic
    rpc_dynamic
    keys_dynamic
    )
target_link_libraries(${prj_base}_static
    rpc_static
    keys_static
    )

set(${prj_base}_includes
    ../include/${prj_base}
    ${CMAKE_SOURCE_DIR}/utils/include
    ${CMAKE_SOURCE_DIR}/keys/include
    ${CMAKE_SOURCE_DIR}/rpc/include    
    )

target_include_directories(${prj_base}_dynamic 
    PRIVATE 
    ${${prj_base}_includes}
    )

target_include_directories(${prj_base}_static 
    PRIVATE 
    ${${prj_base}_includes}
    )

set_target_properties(${prj_base}_dynamic PROPERTIES OUTPUT_NAME "hdac_${prj_base}")
set_target_properties(${prj_base}_dynamic PROPERTIES VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} SOVERSION ${PROJECT_VERSION_MAJOR})

set_target_properties(${prj_base}_static PROPERTIES OUTPUT_NAME "hdac_${prj_base}")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install(TARGETS ${prj_base}_dynamic ${prj_base}_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#file(GLOB headers ../include/${prj_base}/*.h )
set(export_headers
    ../include/${prj_base}/hs_helpers.h
    ../include/${prj_base}/KeysHelperWithRpc.h
    ../include/${prj_base}/KeysHelperWithFile.h
    )
#install(FILES ${headers} DESTINATION include/hdac/${prj_base})
install(FILES ${export_headers} DESTINATION include/hdac/${prj_base})
install(FILES ${CMAKE_BINARY_DIR}/${prj_base}/src/hs_${prj_base}.pc DESTINATION lib/pkgconfig)
