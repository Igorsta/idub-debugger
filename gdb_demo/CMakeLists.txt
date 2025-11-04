file(GLOB SRC_PACK "gdb_demo.cpp")
add_executable(gdb_demo ${SRC_PACK})
target_compile_options(gdb_demo PRIVATE -std=c++23)
