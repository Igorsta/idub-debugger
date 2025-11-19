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

file(GLOB SRC_TIME "src/ioio.cpp")
add_executable(ioio ${SRC_TIME})
target_compile_options(ioio PRIVATE -std=c++23 -O0)


file(GLOB_RECURSE MACRO_SOURCES
    CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/macro_magic/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/macro_magic/*.hpp"	
	"${CMAKE_CURRENT_SOURCE_DIR}/macro_magic/**/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/macro_magic/**/*.hpp"
)
add_executable(macro_magic ${MACRO_SOURCES})

target_include_directories(macro_magic PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/macro_magic
)
