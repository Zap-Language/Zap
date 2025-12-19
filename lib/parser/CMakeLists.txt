include(FetchContent)

# чтобы енамы по именам отображать
FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
)
FetchContent_MakeAvailable(magic_enum)

add_library(parser STATIC
        parser.cpp
        SemanticAnalyzer.cpp
        ast/Program.h
        ast/LetStatement.h
        ast/Identifier.h
        ast/DataType.h
        ast/FuncStatement.h
        ast/DataTypeInt.h
        ast/IntLiteral.h
        ast/DataTypeFunc.h
        ast/PrefixExpression.h
        ast/InfixExpression.h
        ast/DataTypeVoid.h
        ast/BlockStatement.h
        ast/ArgumentList.h
        utils.h
        utils.cpp
        ast/FuncExpression.h
        ast/DataTypeBool.h
        ast/DataTypeFloat.h
        ast/DataTypeChar.h
        ast/DataTypeString.h
        ast/DataTypeArray.h
        ast/BoolLiteral.h
        ast/FloatLiteral.h
        ast/StringLiteral.h
        ast/CharLiteral.h
        ast/ReturnStatement.h
        ast/CallExpression.h
        ast/IfStatement.h
        ast/ElseStatement.h
        ast/WhileStatement.h
        ast/ForStatement.h
        ast/ExpressionStatement.h
        ast/symbols/Scope.cpp
        ast/symbols/SymbolTable.cpp
        ast/AssignStatement.h
)

target_link_libraries(parser
        PRIVATE lexer
        PRIVATE magic_enum::magic_enum
)

target_link_directories(parser
        PUBLIC ../
)

target_include_directories(parser
        PRIVATE .
        PUBLIC ..
)

add_subdirectory(tests)