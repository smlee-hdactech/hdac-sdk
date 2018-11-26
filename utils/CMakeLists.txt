cmake_minimum_required(VERSION 3.1)
project(HDAC_UTILS)

set(REL_INC_PATH "hdac/utils")
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOW)
set(${PROJECT_NAME}_VERSION_MAJOR 0 CACHE STRING "major version of memory" FORCE)
set(${PROJECT_NAME}_VERSION_MINOR 1 CACHE STRING "minor version of memory" FORCE)
set(${PROJECT_NAME}_VERSION_PATCH 0 CACHE STRING "patch version of memory" FORCE)
set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
                             CACHE STRING "version of ${PROJECT_NAME_LOW}" FORCE)

# set a debug postfix
set(CMAKE_DEBUG_POSTFIX "-dbg")

macro(print_variables name)
    get_cmake_property(_variableNames '${ARGV0}')
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endmacro()

# installation destinations
#print_variables(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#    message("check")
#    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "Install prefix (e.g. /usr/local/)" FORCE)
#endif()

#if(UNIX)
#    message("check")
include(GNUInstallDirs)

set(${PROJECT_NAME}_INC_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${REL_INC_PATH}")
set(${PROJECT_NAME}_RUNTIME_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
set(${PROJECT_NAME}_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set(${PROJECT_NAME}_ARCHIVE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set(${PROJECT_NAME}_FRAMEWORK_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")

set(${PROJECT_NAME}_CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME_LOW}/cmake")
set(${PROJECT_NAME}_ADDITIONAL_FILES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME_LOW}")

set(${PROJECT_NAME}_RUNTIME_INSTALL_DIR "bin") # for the nodesize_dbg, just ignore version and the like
set(${PROJECT_NAME}_INC_INSTALL_DIR "include/${REL_INC_PATH}") # header filestree
#endif()

# subdirectories
add_subdirectory(src)
if(${PROJECT_NAME}_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
if(${PROJECT_NAME}_BUILD_TESTS)
    add_subdirectory(test)
endif()
if(${PROJECT_NAME}_BUILD_TOOLS)
    add_subdirectory(tool)
endif()

# install readme and license
#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION ${HDAC_UTILS_ADDITIONAL_FILES_INSTALL_DIR})

install(EXPORT hdac_utilsTargets DESTINATION ${${PROJECT_NAME}_CMAKE_CONFIG_INSTALL_DIR}
                                               FILE ${PROJECT_NAME_LOW}-config.cmake)
