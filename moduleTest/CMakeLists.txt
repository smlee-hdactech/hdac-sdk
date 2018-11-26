cmake_minimum_required(VERSION 3.1)

project(MODULE_TEST)

set(CMAKE_DEBUG_POSTFIX "-dbg")

include(GNUInstallDirs)

set(${PROJECT_NAME}_RUNTIME_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
set(${PROJECT_NAME}_RUNTIME_INSTALL_DIR "bin") # for the nodesize_dbg, just ignore version and the like

# subdirectories
add_subdirectory(src)
