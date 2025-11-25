cmake_minimum_required(VERSION 3.25)

project(IDUB_Swiss_VM CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(common_code INTERFACE)

target_include_directories(common_code INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

file(GLOB_RECURSE MAIN_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/vm/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vm/*.hpp"
)

add_executable(my_vm ${MAIN_SOURCES})

target_include_directories(my_vm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vm
)

target_link_libraries(my_vm PRIVATE common_code)

enable_testing()

add_subdirectory(experimental)
add_subdirectory(gdb_demo)
add_subdirectory(tests)