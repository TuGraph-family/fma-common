cmake_minimum_required(VERSION 3.5)

project(fma-common)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

set(LIB pthread)
if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
    set(LIB ${LIB} stdc++fs)
else()
    set(LIB ${LIB} boost_filesystem)
endif()
link_libraries(${LIB})
include_directories(fma-common)
include_directories(.)

add_subdirectory(test)
