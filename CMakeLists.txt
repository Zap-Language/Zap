cmake_minimum_required(VERSION 3.10)

project(Zap)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(${PROJECT_NAME} main.cpp
        lib/parser/ast.h)

add_subdirectory(lib/lexer)

target_link_libraries(${PROJECT_NAME} PRIVATE lexer)
target_include_directories(${PROJECT_NAME} PRIVATE . ./lib)