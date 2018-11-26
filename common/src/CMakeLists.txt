# builds actual library

set(header_path ${${PROJECT_NAME}_SOURCE_DIR}/include/${REL_INC_PATH})

file(GLOB_RECURSE INCS ${header_path}/*.h)
set(header
    ${INCS}
    )
#message(${header})

set(src
    )
#message(${src})

set(lib_dest "lib/keys")
set(include_dest "include")
set(tool_dest "bin")

include(${CMAKE_SOURCE_DIR}/cmake/external.cmake)
add_library(hdac_common SHARED ${header} ${src})
target_include_directories(hdac_common
    PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include/> # for client in subdirectory
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for generated files in build mode
    $<INSTALL_INTERFACE:${${PROJECT_NAME}_INC_INSTALL_DIR}> # for client in install mode
    PRIVATE
    ${header_path}) # for source files

target_link_libraries(hdac_common
    PUBLIC
    hdac_structs
    ${SECP256K1_LIBRARIES}
    )

target_compile_definitions(hdac_common
    PUBLIC
    ${PROJECT_NAME}=1
    ${PROJECT_NAME}_VERSION_MAJOR=${${PROJECT_NAME}_VERSION_MAJOR}
    ${PROJECT_NAME}_VERSION_MINOR=${${PROJECT_NAME}_VERSION_MINOR}
    ${PROJECT_NAME}_VERSION_PATCH=${${PROJECT_NAME}_VERSION_PATCH})

set_target_properties(hdac_common PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME_LOW}-${${PROJECT_NAME}_VERSION}")

install(TARGETS hdac_common EXPORT hdac_commonTargets
        RUNTIME       DESTINATION ${${PROJECT_NAME}_RUNTIME_INSTALL_DIR}
        LIBRARY       DESTINATION ${${PROJECT_NAME}_LIBRARY_INSTALL_DIR}
        ARCHIVE       DESTINATION ${${PROJECT_NAME}_ARCHIVE_INSTALL_DIR}
        FRAMEWORK     DESTINATION ${${PROJECT_NAME}_FRAMEWORK_INSTALL_DIR})

# Write/install version file
include(CMakePackageConfigHelpers)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME_LOW}-version.cmake")
write_basic_package_version_file(${version_file}
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)


#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config_impl.hpp
#    DESTINATION ${${PROJECT_NAME}_INC_INSTALL_DIR})
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/container_node_sizes_impl.hpp
#    DESTINATION ${HDAC_UTILS_INC_INSTALL_DIR}/foonathan/memory/detail)
install(FILES ${header}
    DESTINATION ${${PROJECT_NAME}_INC_INSTALL_DIR}/)
#install(FILES ${detail_header}
#    DESTINATION ${HDAC_UTILS_INC_INSTALL_DIR}/hdac/utils/detail)
install(FILES ${version_file}
    DESTINATION ${${PROJECT_NAME}_CMAKE_CONFIG_INSTALL_DIR})

