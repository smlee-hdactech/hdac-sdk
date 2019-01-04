cmake_minimum_required(VERSION 3.1)

project(HDAC_UTILS)

set(PROJECT_DESCRIPTION "the utility function related to encoding primarily")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

add_subdirectory(src)
