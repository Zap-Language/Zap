#include <gtest/gtest.h>
#include <chrono>
#include <functional>
#include <memory>
#include "vm.h"
#include "jit.h"
#include "../lib/bytecode/compiler.h"
#include "../lib/parser/Parser.h"
#include "../lib/lexer/lexer.h"
#include "../lib/parser/SemanticAnalyzer.h"

using namespace jit;

const char* testCode = R"(
func addNumbers(int a, int b) int {
    return a + b
}

func multiply(int x, int y) int {
    return x * y
}

let result1 = addNumbers(10, 20)
let result2 = multiply(5, 6)
)";

const int ITERATIONS = 100000;

std::unique_ptr<bytecode::BytecodeChunk> compileProgram(const std::string& code) {
    std::string codeCopy = code;
    Lexer lexer(codeCopy);
    ast::Parser parser(lexer);
    auto program = parser.ParseProgram();
    
    if (!program || !parser._errors.empty()) {
        return nullptr;
    }
    
    ast::SemanticAnalyzer analyzer;
    std::shared_ptr<ast::Program> sharedProgram(program.release());
    bool success = analyzer.AnalyzeProgram(sharedProgram);
    
    if (!success || !analyzer.GetErrors().empty()) {
        return nullptr;
    }
    
    bytecode::Compiler compiler;
    return compiler.Compile(*sharedProgram);
}

template<typename Func>
int64_t measureTime(Func func, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        func();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

std::unique_ptr<bytecode::BytecodeChunk> cloneChunk(const bytecode::BytecodeChunk& original) {
    auto clone = std::make_unique<bytecode::BytecodeChunk>();
    
    for (uint8_t byte : original.Code()) {
        clone->EmitByte(byte);
    }
    
    for (const auto& func : original.Functions()) {
        clone->AddFunction(func);
    }
    
    for (const auto& str : original.Strings()) {
        clone->AddString(str);
    }
    
    return clone;
}

TEST(JITPerformanceTest, FunctionCallsInterpreterVsJIT) {
    std::string testCodeWithManyCalls = R"(
func addNumbers(int a, int b) int {
    return a + b
}

let r1 = addNumbers(10, 20)
let r2 = addNumbers(10, 20)
let r3 = addNumbers(10, 20)
let r4 = addNumbers(10, 20)
let r5 = addNumbers(10, 20)
let r6 = addNumbers(10, 20)
let r7 = addNumbers(10, 20)
let r8 = addNumbers(10, 20)
let r9 = addNumbers(10, 20)
let r10 = addNumbers(10, 20)
let r11 = addNumbers(10, 20)
let r12 = addNumbers(10, 20)
let r13 = addNumbers(10, 20)
let r14 = addNumbers(10, 20)
let r15 = addNumbers(10, 20)
)";
    
    auto originalChunk = compileProgram(testCodeWithManyCalls);
    ASSERT_NE(originalChunk, nullptr);
    
    std::cout << "Functions called 15 times per execution" << std::endl;
    
    int64_t interpreterTime = measureTime([&originalChunk]() {
        auto chunk = cloneChunk(*originalChunk);
        VM vm;
        vm.loadChunk(std::move(chunk));
        vm.run();
    }, ITERATIONS);
    
    VM jitVM;
    JITCompiler jit(&jitVM);
    jitVM.setJITCompiler(&jit);
    
    {
        auto warmupChunk = cloneChunk(*originalChunk);
        jitVM.loadChunk(std::move(warmupChunk));
        jitVM.run();
    }
    
    std::cout << "JIT compiled functions: " << jit.getCompiledFunctionsCount() << std::endl;
    
    int64_t jitTime = measureTime([&originalChunk, &jitVM]() {
        auto chunk = cloneChunk(*originalChunk);
        jitVM.loadChunk(std::move(chunk));
        jitVM.run();
    }, ITERATIONS);
    
    double speedup = static_cast<double>(interpreterTime) / jitTime;
    
    std::cout << "Interpreter: " << interpreterTime << " us (" 
              << (interpreterTime / 1000.0) << " ms)" << std::endl;
    std::cout << "JIT: " << jitTime << " us (" 
              << (jitTime / 1000.0) << " ms)" << std::endl;
    std::cout << "Speedup: " << speedup << "x" << std::endl;
    
    EXPECT_GT(jit.getCompiledFunctionsCount(), 0);
    EXPECT_GT(speedup, 0.9);
}

TEST(JITPerformanceTest, IntensiveFunctionCalls) {
    std::string intensiveCode = R"(
func compute(int a, int b) int {
    return a + b
}
)";
    
    for (int i = 0; i < 100; ++i) {
        intensiveCode += "let x" + std::to_string(i) + " = compute(1, 2)\n";
    }
    
    auto originalChunk = compileProgram(intensiveCode);
    ASSERT_NE(originalChunk, nullptr);
    
    const int iters = 5000;
    
    std::cout << "100 function calls per execution, " << iters << " iterations" << std::endl;
    
    int64_t interpreterTime = measureTime([&originalChunk]() {
        auto chunk = cloneChunk(*originalChunk);
        VM vm;
        vm.loadChunk(std::move(chunk));
        vm.run();
    }, iters);
    
    VM jitVM;
    JITCompiler jit(&jitVM);
    jitVM.setJITCompiler(&jit);
    
    {
        auto warmupChunk = cloneChunk(*originalChunk);
        jitVM.loadChunk(std::move(warmupChunk));
        jitVM.run();
    }
    
    std::cout << "JIT compiled functions: " << jit.getCompiledFunctionsCount() << std::endl;
    
    int64_t jitTime = measureTime([&originalChunk, &jitVM]() {
        auto chunk = cloneChunk(*originalChunk);
        jitVM.loadChunk(std::move(chunk));
        jitVM.run();
    }, iters);
    
    double speedup = static_cast<double>(interpreterTime) / jitTime;
    
    std::cout << "Interpreter: " << interpreterTime << " us (" 
              << (interpreterTime / 1000.0) << " ms)" << std::endl;
    std::cout << "JIT: " << jitTime << " us (" 
              << (jitTime / 1000.0) << " ms)" << std::endl;
    std::cout << "Speedup: " << speedup << "x" << std::endl;
    
    EXPECT_GT(jit.getCompiledFunctionsCount(), 0);
}
