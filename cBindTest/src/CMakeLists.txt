cmake_minimum_required(VERSION 3.1)

add_executable(hdac_cBindTest main.c)

target_include_directories(hdac_cBindTest 
    PRIVATE
    ${CMAKE_SOURCE_DIR}/keys/include
    )

if(UNIX)
target_link_libraries(hdac_cBindTest
    PUBLIC
    keys_wrapper
    Boost::thread
    Boost::system   
    )
else()
target_link_libraries(hdac_cBindTest
    PUBLIC
    keys_wrapper
    Boost::thread
    Boost::system   
    )
endif()

#message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
#set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin) 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#message("PROJECT_BINARY_DIR="${PROJECT_BINARY_DIR})
#message("CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
add_custom_command(TARGET hdac_cBindTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${PROJECT_BINARY_DIR}/../keys/lib/keys_wrapper.dll
	${EXECUTABLE_OUTPUT_PATH})
endif()