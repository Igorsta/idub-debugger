link_libraries(common_code)

file(GLOB SRC_PACK "src/sanity_check.cpp")
add_executable(sanity_check ${SRC_PACK})
target_compile_options(sanity_check PRIVATE -std=c++23)

file(GLOB SRC_TIME "src/achrono.cpp")
add_executable(achrono ${SRC_TIME})
target_compile_options(achrono PRIVATE -std=c++23)

file(GLOB SRC_TIME "src/exec_termination.cpp")
add_executable(exec_term ${SRC_TIME})
target_compile_options(exec_term PRIVATE -std=c++23 -O3)
