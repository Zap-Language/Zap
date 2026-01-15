add_library(jit_lib STATIC
    gc.cpp
    vm.cpp
    jit.cpp
)

target_include_directories(jit_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(jit_lib
    PUBLIC bytecode
    PUBLIC parser
    PUBLIC lexer
    PUBLIC magic_enum::magic_enum
)

add_executable(zap
    main.cpp
)

target_include_directories(zap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(zap
    PRIVATE jit_lib
)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(jit_test
    test_gc_vm.cpp
)

target_include_directories(jit_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/bytecode
    ${CMAKE_SOURCE_DIR}/lib/parser
    ${CMAKE_SOURCE_DIR}/lib/lexer
)

target_link_libraries(jit_test
    PRIVATE jit_lib
    PRIVATE GTest::gtest_main
)

add_test(NAME jit_test COMMAND jit_test)

add_executable(jit_performance_test
    test_jit_performance.cpp
)

target_include_directories(jit_performance_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/bytecode
    ${CMAKE_SOURCE_DIR}/lib/parser
    ${CMAKE_SOURCE_DIR}/lib/lexer
)

target_link_libraries(jit_performance_test
    PRIVATE jit_lib
    PRIVATE bytecode
    PRIVATE parser
    PRIVATE lexer
    PRIVATE GTest::gtest_main
)

add_test(NAME jit_performance_test COMMAND jit_performance_test)

set_target_properties(jit_performance_test PROPERTIES
    FOLDER "Tests"
)

set_target_properties(jit_test PROPERTIES
    FOLDER "Tests"
)
