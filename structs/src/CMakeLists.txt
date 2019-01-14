cmake_minimum_required(VERSION 3.1)

set(prj_base structs)

configure_file(hs_${prj_base}.pc.in hs_${prj_base}.pc @ONLY)

set(src
    base58.cpp
    hash.cpp
    uint256.cpp
    hashes.cpp
    hs_structs.cpp
    )

if(UNIX)
add_library(${prj_base}_dynamic SHARED ${src})
target_link_libraries(${prj_base}_dynamic
    utils_dynamic
    )
target_include_directories(${prj_base}_dynamic 
    PRIVATE 
    ../include/${prj_base}
    ${CMAKE_SOURCE_DIR}/utils/include
    ${CMAKE_SOURCE_DIR}/crypto/include)

set_target_properties(${prj_base}_dynamic PROPERTIES OUTPUT_NAME "hdac_${prj_base}")
set_target_properties(${prj_base}_dynamic PROPERTIES VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} SOVERSION ${PROJECT_VERSION_MAJOR})

endif()

add_library(${prj_base}_static STATIC ${src})
target_link_libraries(${prj_base}_static
    utils_static
    )

target_include_directories(${prj_base}_static 
    PRIVATE 
    ../include/${prj_base}
    ${CMAKE_SOURCE_DIR}/utils/include
    ${CMAKE_SOURCE_DIR}/crypto/include)

set_target_properties(${prj_base}_static PROPERTIES OUTPUT_NAME "hdac_${prj_base}")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(UNIX)
install(TARGETS ${prj_base}_dynamic ${prj_base}_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
else()
install(TARGETS ${prj_base}_static        
        ARCHIVE DESTINATION lib)
endif()        

file(GLOB headers ../include/${prj_base}/*.h )
#install(FILES ${headers} DESTINATION include/hdac/${prj_base})
install(FILES ${CMAKE_BINARY_DIR}/${prj_base}/src/hs_${prj_base}.pc DESTINATION lib/pkgconfig)
