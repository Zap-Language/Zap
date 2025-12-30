cmake_minimum_required(VERSION 3.10)

project(Zap)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_executable(${PROJECT_NAME} main.cpp
)

add_subdirectory(lib/lexer)
add_subdirectory(lib/parser)
add_subdirectory(lib/bytecode)
add_subdirectory(jit)

target_link_libraries(${PROJECT_NAME}
        PRIVATE lexer
        PRIVATE parser
        PRIVATE bytecode
        PRIVATE jit_lib
)
target_include_directories(${PROJECT_NAME} PRIVATE . ./lib)