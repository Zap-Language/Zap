FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(compiler_test
        compiler_test.cpp
)

target_link_libraries(compiler_test
        PRIVATE parser
        PRIVATE lexer
        PRIVATE bytecode
        GTest::gtest_main
)

target_include_directories(compiler_test PUBLIC . ../.. ${PROJECT_SOURCE_DIR}/lib)
add_test(NAME compiler_test COMMAND compiler_test)
