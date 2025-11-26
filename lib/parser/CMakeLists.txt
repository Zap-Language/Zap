add_library(parser STATIC
        parser.cpp
        ast/program.h
        ast/LetStatement.h
        ast/Identifier.h
        ast/DataType.h
        ast/FuncStatement.h
        ast/DataTypeInt.h
)

target_link_libraries(parser
        PRIVATE lexer
)

target_link_directories(parser
        PRIVATE ../
)

target_include_directories(parser
        PRIVATE .
        PRIVATE ..
)