#pragma once

#include "gc.h"
#include "value.h"
#include "../lib/bytecode/bytecode_chunk.h"
#include "../lib/bytecode/opcodes.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <memory>

namespace jit {

class JITCompiler;

struct CallFrame {
    uint32_t functionIndex;
    size_t ip;
    size_t stackStart;
    uint8_t localCount;
};

class VM {
public:
    VM();
    ~VM();

    void loadChunk(std::unique_ptr<bytecode::BytecodeChunk> chunk);
    void run();
    
    void setJITCompiler(class JITCompiler* jit) { jitCompiler = jit; }

    GarbageCollector& getGC() { return gc; }
    
    const bytecode::BytecodeChunk* getChunk() const { return chunk.get(); }
    bytecode::BytecodeChunk* getChunk() { return chunk.get(); }

    enum class Status {
        OK,
        RUNTIME_ERROR,
        STACK_OVERFLOW,
        STACK_UNDERFLOW,
    };

    Status getStatus() const { return status; }

    static constexpr size_t STACK_MAX = 256;
    std::vector<Value> stack;
    std::vector<Value> globals;
    std::stack<CallFrame> callStack;

    void executeInstruction(bytecode::OpCode op);
    void call();
    void callBuiltin();
    void returnOp();
    void returnVoid();
    void newArray();
    void arrayGet();
    void arraySet();
    void arrayLen();
    void divide();
    void modulo();
    void power();
    void pushString();
    void pushFunc();
    void halt();

private:
    std::unique_ptr<bytecode::BytecodeChunk> chunk;
    size_t ip;
    GarbageCollector gc;
    class JITCompiler* jitCompiler;
    Status status;

    void push(const Value& value);
    Value pop();
    Value peek(size_t distance = 0) const;
    
    friend class JITCompiler;

    void registerRoots();
    
    void pushInt();
    void pushFloat();
    void pushBool();
    void pushChar();
    void pushNull();
    
    void loadLocal();
    void storeLocal();
    void loadGlobal();
    void storeGlobal();
    
    void add();
    void subtract();
    void multiply();
    void negate();
    
    void andOp();
    void orOp();
    void notOp();
    
    void cmpEq();
    void cmpNe();
    void cmpLt();
    void cmpGt();
    void cmpLe();
    void cmpGe();
    
    void jump();
    void jumpIfFalse();
    void jumpIfTrue();
    
    void popOp();
    void dup();

    void builtinPrint();
    void builtinLen();
    void builtinRead();
};

}
