file(GLOB SOURCES "pack.cpp")

add_executable(pack_index ${SOURCES})
target_compile_options(pack_index PRIVATE -std=c++26)

