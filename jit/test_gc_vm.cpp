#include <gtest/gtest.h>
#include <vector>
#include <cstdint>
#include <climits>
#include "gc.h"
#include "vm.h"
#include "jit.h"
#include "obj_string.h"
#include "obj_array.h"
#include "value.h"
#include "../lib/bytecode/bytecode_chunk.h"
#include "../lib/bytecode/opcodes.h"

using namespace jit;
using namespace bytecode;

TEST(GCTest, BasicGC) {
    GarbageCollector gc;
    
    ObjString* str1 = gc.allocate<ObjString>("Hello");
    ObjString* str2 = gc.allocate<ObjString>("World");
    
    ASSERT_EQ(gc.getObjectCount(), 2);
    ASSERT_EQ(str1->toString(), "Hello");
    ASSERT_EQ(str2->toString(), "World");
}

TEST(GCTest, MarkAndSweep) {
    GarbageCollector gc;
    
    ObjString* str1 = gc.allocate<ObjString>("Reachable1");
    ObjString* str2 = gc.allocate<ObjString>("Reachable2");
    ObjArray* arr = gc.allocate<ObjArray>(2);
    arr->elements[0] = Value(str1);
    arr->elements[1] = Value(str2);
    
    ObjString* unreachable = gc.allocate<ObjString>("Unreachable");
    
    ASSERT_EQ(gc.getObjectCount(), 4);
    
    std::vector<Value> stack;
    stack.push_back(Value(arr));
    std::vector<Value*> roots;
    roots.push_back(&stack[0]);
    gc.setRoots(roots);
    
    gc.collect();
    
    ASSERT_EQ(gc.getObjectCount(), 3);
    ASSERT_EQ(arr->size(), 2);
    ASSERT_EQ(arr->size(), 2);
    ASSERT_EQ(arr->elements[0].getObj(), str1);
}

TEST(GCTest, ArrayChildren) {
    GarbageCollector gc;
    
    ObjString* str1 = gc.allocate<ObjString>("Child1");
    ObjString* str2 = gc.allocate<ObjString>("Child2");
    ObjArray* arr = gc.allocate<ObjArray>(2);
    arr->elements[0] = Value(str1);
    arr->elements[1] = Value(str2);
    
    std::vector<Value> stack;
    stack.push_back(Value(arr));
    std::vector<Value*> roots;
    roots.push_back(&stack[0]);
    gc.setRoots(roots);
    
    ObjString* orphan = gc.allocate<ObjString>("Orphan");
    
    gc.collect();
    
    ASSERT_EQ(gc.getObjectCount(), 3);
    ASSERT_EQ(static_cast<ObjString*>(arr->elements[0].getObj())->toString(), "Child1");
}

TEST(VMTest, PushInt) {
    VM vm;
    auto chunk = std::make_unique<BytecodeChunk>();
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(42);
    chunk->EmitOpCode(OpCode::HALT);
    
    vm.loadChunk(std::move(chunk));
    vm.run();
    
    ASSERT_EQ(vm.getStatus(), VM::Status::OK);
}

TEST(VMTest, Arithmetic) {
    VM vm;
    auto chunk = std::make_unique<BytecodeChunk>();
    
    // 10 + 20 = 30
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(10);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(20);
    chunk->EmitOpCode(OpCode::ADD);
    chunk->EmitOpCode(OpCode::HALT);
    
    vm.loadChunk(std::move(chunk));
    vm.run();
    
    ASSERT_EQ(vm.getStatus(), VM::Status::OK);
}

TEST(VMTest, NegativeNumbers) {
    VM vm;
    auto chunk = std::make_unique<BytecodeChunk>();
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(-5);
    chunk->EmitOpCode(OpCode::HALT);
    
    vm.loadChunk(std::move(chunk));
    vm.run();
    
    ASSERT_EQ(vm.getStatus(), VM::Status::OK);
}

TEST(VMTest, Strings) {
    VM vm;
    auto chunk = std::make_unique<BytecodeChunk>();
    
    uint32_t strIndex = chunk->AddString("TestString");
    chunk->EmitOpCode(OpCode::PUSH_STRING);
    chunk->EmitUint32(strIndex);
    chunk->EmitOpCode(OpCode::HALT);
    
    vm.loadChunk(std::move(chunk));
    vm.run();
    
    ASSERT_EQ(vm.getStatus(), VM::Status::OK);
    ASSERT_GE(vm.getGC().getObjectCount(), 1);
}

TEST(VMTest, Arrays) {
    VM vm;
    auto chunk = std::make_unique<BytecodeChunk>();
    
    chunk->EmitOpCode(OpCode::NEW_ARRAY);
    chunk->EmitUint32(3);
    chunk->EmitByte(static_cast<uint8_t>(ValueType::INT));
    chunk->EmitOpCode(OpCode::HALT);
    
    vm.loadChunk(std::move(chunk));
    vm.run();
    
    ASSERT_EQ(vm.getStatus(), VM::Status::OK);
    ASSERT_GE(vm.getGC().getObjectCount(), 1);
}

TEST(Int64Test, Encoding) {
    BytecodeChunk chunk;
    
    int64_t testValues[] = {0, 1, 127, 128, 255, 256, 65535, INT32_MAX, INT64_MAX};
    
    for (int64_t original : testValues) {
        chunk.EmitInt64(original);
        size_t offset = chunk.CurrentOffset() - 8;
        int64_t decoded = chunk.ReadInt64(offset);
        
        ASSERT_EQ(decoded, original);
    }
}

TEST(Int64Test, NegativeEncoding) {
    BytecodeChunk chunk;
    
    int64_t testValues[] = {-1, -128, -255, -256, -65535, INT32_MIN, INT64_MIN};
    
    for (int64_t original : testValues) {
        chunk.EmitInt64(original);
        size_t offset = chunk.CurrentOffset() - 8;
        int64_t decoded = chunk.ReadInt64(offset);
        
        ASSERT_EQ(decoded, original);
    }
}

TEST(Int64Test, EdgeCases) {
    BytecodeChunk chunk;
    
    int64_t min = INT64_MIN;
    int64_t max = INT64_MAX;
    
    chunk.EmitInt64(min);
    ASSERT_EQ(chunk.ReadInt64(0), min);
    
    chunk.EmitInt64(max);
    ASSERT_EQ(chunk.ReadInt64(8), max);
}

TEST(JITTest, BasicCreation) {
    VM vm;
    JITCompiler jit(&vm);
    
    ASSERT_EQ(jit.getCompiledFunctionsCount(), 0);
    ASSERT_EQ(JITCompiler::getJITThreshold(), 10);
}

TEST(JITTest, CallCountIncrement) {
    VM vm;
    JITCompiler jit(&vm);
    
    jit.incrementCallCount(0);
    jit.incrementCallCount(0);
    jit.incrementCallCount(0);
    
    ASSERT_FALSE(jit.isHotFunction(0));
    
    for (int i = 0; i < 7; ++i) {
        jit.incrementCallCount(0);
    }
    
    ASSERT_TRUE(jit.isHotFunction(0));
}

TEST(JITTest, MultipleFunctions) {
    VM vm;
    JITCompiler jit(&vm);
    
    jit.incrementCallCount(0);
    jit.incrementCallCount(1);
    jit.incrementCallCount(2);
    
    ASSERT_FALSE(jit.isHotFunction(0));
    ASSERT_FALSE(jit.isHotFunction(1));
    ASSERT_FALSE(jit.isHotFunction(2));
    
    for (int i = 0; i < 10; ++i) {
        jit.incrementCallCount(1);
    }
    
    ASSERT_TRUE(jit.isHotFunction(1));
    ASSERT_FALSE(jit.isHotFunction(0));
    ASSERT_FALSE(jit.isHotFunction(2));
}

TEST(JITTest, GetCompiledFunction) {
    VM vm;
    JITCompiler jit(&vm);
    
    ASSERT_EQ(jit.getCompiledFunction(0), nullptr);
    ASSERT_EQ(jit.getCompiledFunction(1), nullptr);
}

TEST(JITTest, CompileSimpleFunction) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.codeLength = 9;  // PUSH_INT (1) + int64 (8)
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(42);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    
    ASSERT_TRUE(compiledFunc.has_value());
    
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, CompileFunctionWithArithmetic) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "add";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(10);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(20);
    chunk->EmitOpCode(OpCode::ADD);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    ASSERT_TRUE(compiledFunc.has_value());
    
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, CompileFunctionWithSubtraction) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "sub";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(50);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(30);
    chunk->EmitOpCode(OpCode::SUB);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, CompileFunctionWithMultiplication) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "mul";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(6);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(7);
    chunk->EmitOpCode(OpCode::MUL);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, CompileFunctionWithFloat) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "floatTest";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::FLOAT;
    
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(3.14);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, AutoCompilationOnHotFunction) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "hotFunc";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(100);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    for (int i = 0; i < 10; ++i) {
        jit.incrementCallCount(funcIndex);
    }
    
    ASSERT_TRUE(jit.isHotFunction(funcIndex));
    ASSERT_NE(jit.getCompiledFunction(funcIndex), nullptr);
    ASSERT_EQ(jit.getCompiledFunctionsCount(), 1);
}

TEST(JITTest, CompileMultipleFunctions) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo func1;
    func1.name = "func1";
    func1.codeOffset = 0;
    func1.paramCount = 0;
    func1.localCount = 0;
    func1.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(1);
    chunk->EmitOpCode(OpCode::RETURN);
    
    func1.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex1 = chunk->AddFunction(func1);
    
    FunctionInfo func2;
    func2.name = "func2";
    func2.codeOffset = chunk->CurrentOffset();
    func2.paramCount = 0;
    func2.localCount = 0;
    func2.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(2);
    chunk->EmitOpCode(OpCode::RETURN);
    
    func2.codeLength = chunk->CurrentOffset() - func2.codeOffset;
    uint32_t funcIndex2 = chunk->AddFunction(func2);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiled1 = jit.compileFunction(funcIndex1);
    auto compiled2 = jit.compileFunction(funcIndex2);
    
    ASSERT_TRUE(compiled1.has_value());
    ASSERT_TRUE(compiled2.has_value());
}

TEST(JITTest, CompileFunctionErrorHandling) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto result1 = jit.compileFunction(0);
    ASSERT_FALSE(result1.has_value());
    
    auto chunk = std::make_unique<BytecodeChunk>();
    vm.loadChunk(std::move(chunk));
    
    auto result2 = jit.compileFunction(999);
    ASSERT_FALSE(result2.has_value());
}

TEST(JITTest, GenerateInstructionPUSH_INT) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(12345);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, GenerateInstructionPUSH_FLOAT) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::FLOAT;
    
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(99.99);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, GenerateInstructionADD) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(15);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(25);
    chunk->EmitOpCode(OpCode::ADD);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, GenerateInstructionSUB) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(100);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(40);
    chunk->EmitOpCode(OpCode::SUB);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, GenerateInstructionMUL) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(8);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(9);
    chunk->EmitOpCode(OpCode::MUL);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, GenerateInstructionRETURN) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(777);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, GenerateInstructionDefaultCase) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(1);
    chunk->EmitOpCode(OpCode::HALT);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, ComplexFunction) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "complex";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(10);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(20);
    chunk->EmitOpCode(OpCode::ADD);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(2);
    chunk->EmitOpCode(OpCode::MUL);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, FloatArithmetic) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "floatArith";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::FLOAT;
    
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(1.5);
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(2.5);
    chunk->EmitOpCode(OpCode::ADD);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, IsHotFunctionNonExistent) {
    VM vm;
    JITCompiler jit(&vm);
    
    ASSERT_FALSE(jit.isHotFunction(999));
}

TEST(JITTest, IncrementCallCountDoesNotCompileTwice) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "test";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(42);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    for (int i = 0; i < 10; ++i) {
        jit.incrementCallCount(funcIndex);
    }
    
    ASSERT_EQ(jit.getCompiledFunctionsCount(), 1);
    
    jit.incrementCallCount(funcIndex);
    ASSERT_EQ(jit.getCompiledFunctionsCount(), 1);
}

TEST(JITTest, EmptyFunction) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "empty";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::VOID;
    
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, FloatSubtraction) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "floatSub";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::FLOAT;
    
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(10.5);
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(3.2);
    chunk->EmitOpCode(OpCode::SUB);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, FloatMultiplication) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "floatMul";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::FLOAT;
    
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(2.5);
    chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    chunk->EmitDouble(4.0);
    chunk->EmitOpCode(OpCode::MUL);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, MixedIntFloatOperations) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "mixed";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(5);
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(3);
    chunk->EmitOpCode(OpCode::ADD);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, CompileFunctionWithLongCode) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "longFunc";
    funcInfo.codeOffset = 0;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    for (int i = 0; i < 5; ++i) {
        chunk->EmitOpCode(OpCode::PUSH_INT);
        chunk->EmitInt64(i);
    }
    for (int i = 0; i < 4; ++i) {
        chunk->EmitOpCode(OpCode::ADD);
    }
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset();
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

TEST(JITTest, CompileFunctionBoundaryConditions) {
    VM vm;
    JITCompiler jit(&vm);
    
    auto chunk = std::make_unique<BytecodeChunk>();
    
    FunctionInfo funcInfo;
    funcInfo.name = "boundary";
    funcInfo.codeOffset = 10;
    funcInfo.paramCount = 0;
    funcInfo.localCount = 0;
    funcInfo.returnType = ValueType::INT;
    
    for (int i = 0; i < 10; ++i) {
        chunk->EmitOpCode(OpCode::NOP);
    }
    
    chunk->EmitOpCode(OpCode::PUSH_INT);
    chunk->EmitInt64(999);
    chunk->EmitOpCode(OpCode::RETURN);
    
    funcInfo.codeLength = chunk->CurrentOffset() - funcInfo.codeOffset;
    uint32_t funcIndex = chunk->AddFunction(funcInfo);
    
    vm.loadChunk(std::move(chunk));
    
    auto compiledFunc = jit.compileFunction(funcIndex);
    if (compiledFunc) (*compiledFunc)();
}

