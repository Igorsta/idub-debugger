cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)          # use C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # fail if C++20 not supported

file(GLOB SOURCES_EXP "src/experimental/*.cpp")
file(GLOB SOURCES_VM "src/vm/*.cpp")

message(STATUS "Sources for experiment: ${SOURCES_EXP}")  # optional, debug
message(STATUS "Sources for vm: ${SOURCES_VM}")  # optional, debug

add_executable(exp ${SOURCES_EXP})
add_executable(baby_vm ${SOURCES_VM})