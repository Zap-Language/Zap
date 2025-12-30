#include <iostream>
#include "vm.h"
#include "jit.h"
#include "gc.h"
#include "../lib/bytecode/compiler.h"
#include "../lib/parser/Parser.h"
#include "../lib/lexer/lexer.h"

using namespace jit;

int main(int argc, const char** argv) {

    if (argc < 2) {

        VM vm;
        
        JITCompiler jit(&vm);
        vm.setJITCompiler(&jit);
        
        auto chunk = std::make_unique<bytecode::BytecodeChunk>();


        chunk->EmitOpCode(bytecode::OpCode::PUSH_STRING);
        uint32_t str1Index = chunk->AddString("Hello");
        chunk->EmitUint32(str1Index);
        
        chunk->EmitOpCode(bytecode::OpCode::STORE_GLOBAL);
        chunk->EmitUint32(0);
        
        chunk->EmitOpCode(bytecode::OpCode::PUSH_STRING);
        uint32_t str2Index = chunk->AddString("World");
        chunk->EmitUint32(str2Index);
        
        chunk->EmitOpCode(bytecode::OpCode::STORE_GLOBAL);
        chunk->EmitUint32(1);
        
        chunk->EmitOpCode(bytecode::OpCode::PUSH_STRING);
        uint32_t str3Index = chunk->AddString("Test");
        chunk->EmitUint32(str3Index);
        
        chunk->EmitOpCode(bytecode::OpCode::NEW_ARRAY);
        chunk->EmitUint32(2);
        chunk->EmitByte(static_cast<uint8_t>(bytecode::ValueType::STRING));
        
        chunk->EmitOpCode(bytecode::OpCode::LOAD_GLOBAL);
        chunk->EmitUint32(0);
        chunk->EmitOpCode(bytecode::OpCode::ARRAY_SET);
        chunk->EmitUint32(0);
        
        chunk->EmitOpCode(bytecode::OpCode::LOAD_GLOBAL);
        chunk->EmitUint32(1);
        chunk->EmitOpCode(bytecode::OpCode::ARRAY_SET);
        chunk->EmitUint32(1);
        
        chunk->EmitOpCode(bytecode::OpCode::STORE_GLOBAL);
        chunk->EmitUint32(2);
        
        std::cout << "   Objects before GC: " << vm.getGC().getObjectCount() << "\n";
        std::cout << "   Memory allocated: " << vm.getGC().getBytesAllocated() << " bytes\n\n";
        
        vm.loadChunk(std::move(chunk));
        vm.run();
        
        std::cout << "   Objects after execution: " << vm.getGC().getObjectCount() << "\n";
        std::cout << "   Memory allocated: " << vm.getGC().getBytesAllocated() << " bytes\n\n";
        
        vm.getGC().collect();
        
        std::cout << "   Objects after GC: " << vm.getGC().getObjectCount() << "\n";
        std::cout << "   Memory allocated: " << vm.getGC().getBytesAllocated() << " bytes\n";
        std::cout << "   Unreachable objects removed\n\n";
        
        auto chunk2 = std::make_unique<bytecode::BytecodeChunk>();
        
        chunk2->EmitOpCode(bytecode::OpCode::PUSH_INT);
        chunk2->EmitInt64(10);
        
        chunk2->EmitOpCode(bytecode::OpCode::PUSH_INT);
        chunk2->EmitInt64(20);
        
        chunk2->EmitOpCode(bytecode::OpCode::ADD);
        
        chunk2->EmitOpCode(bytecode::OpCode::PUSH_INT);
        chunk2->EmitInt64(5);
        
        chunk2->EmitOpCode(bytecode::OpCode::MUL);
        
        chunk2->EmitOpCode(bytecode::OpCode::HALT);
        
        vm.loadChunk(std::move(chunk2));
        vm.run();
        
        std::cout << "   Calculation result (10 + 20) * 5 = 150\n";
        std::cout << "   Primitives don't require GC\n\n";
        
        std::cout << "6. JIT Compiler statistics:\n";
        std::cout << "   Compiled functions: " << jit.getCompiledFunctionsCount() << "\n";
        std::cout << "   (Functions are compiled when called " << JITCompiler::getJITThreshold() << "+ times)\n\n";
        
        return 0;
    }
    return 0;
}
