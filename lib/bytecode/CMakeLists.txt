add_library(bytecode STATIC
        bytecode_chunk.cpp
        symbol_table.cpp
        compiler.cpp
        disassembler.cpp
)

target_include_directories(bytecode
        PRIVATE .
        PUBLIC ..
)
target_link_directories(bytecode
        PUBLIC ..
)

target_link_libraries(bytecode PRIVATE parser lexer)

add_subdirectory(test)