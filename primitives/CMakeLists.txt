cmake_minimum_required(VERSION 3.1)

project(HDAC_PRIMITIVES)

set(PROJECT_DESCRIPTION "the functions related to transaction")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 2)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

add_subdirectory(src)
add_subdirectory(example)
