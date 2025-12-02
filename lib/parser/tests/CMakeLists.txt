# тестики
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(func_tests
        func_test.cpp
)

target_link_libraries(func_tests
        PRIVATE parser
        PRIVATE lexer
        GTest::gtest_main
)

target_include_directories(func_tests PRIVATE . ..)
add_test(NAME func_tests COMMAND func_tests)
