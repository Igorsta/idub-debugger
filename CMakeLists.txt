cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)          # use C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # fail if C++20 not supported

file(GLOB SOURCES "src/*.cpp")

message(STATUS "Sources: ${SOURCES}")  # optional, debug

add_executable(baby_vm ${SOURCES})