include(FetchContent)

# чтобы енамы по именам отображать
FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
)
FetchContent_MakeAvailable(magic_enum)

# тестики
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(parser STATIC
        parser.cpp
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

enable_testing()

add_executable(func_tests
        tests/func_test.cpp
)

target_link_libraries(func_tests
        PRIVATE parser
        PRIVATE lexer
        GTest::gtest_main
)

target_include_directories(func_tests PRIVATE .)
add_test(NAME func_tests COMMAND func_tests)