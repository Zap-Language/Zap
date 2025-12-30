#pragma once

#include "vm.h"
#include "../lib/bytecode/bytecode_chunk.h"
#include <unordered_map>
#include <functional>
#include <vector>
#include <optional>

namespace jit {

class JITCompiler {
public:
    JITCompiler(VM* vm);
    
    void incrementCallCount(uint32_t functionIndex);
    bool isHotFunction(uint32_t functionIndex) const;
    std::optional<std::function<void()>> compileFunction(uint32_t functionIndex);
    std::function<void()>* getCompiledFunction(uint32_t functionIndex);
    
    size_t getCompiledFunctionsCount() const { return compiledFunctions.size(); }
    size_t getCallCount(uint32_t functionIndex) const;
    static size_t getJITThreshold() { return JIT_THRESHOLD; }
    
private:
    VM* vm;
    std::unordered_map<uint32_t, size_t> callCounts;
    std::unordered_map<uint32_t, std::function<void()>> compiledFunctions;
    static constexpr size_t JIT_THRESHOLD = 10;
    
    bool generateInstruction(bytecode::OpCode op, size_t& codeOffset, 
                            std::vector<std::function<void()>>& code);
};

}
