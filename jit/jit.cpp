#include "jit.h"
#include "vm.h"
#include <iostream>

namespace jit {

JITCompiler::JITCompiler(VM* vm) : vm(vm) {
}

void JITCompiler::incrementCallCount(const uint32_t functionIndex) {
    callCounts[functionIndex]++;
    
    if (isHotFunction(functionIndex) &&
        !compiledFunctions.contains(functionIndex) &&
        vm && vm->getChunk()) {
        if (const auto compiled = compileFunction(functionIndex); compiled.has_value()) {
            compiledFunctions[functionIndex] = compiled.value();
        }
    }
}

size_t JITCompiler::getCallCount(uint32_t functionIndex) const {
    auto it = callCounts.find(functionIndex);
    return (it != callCounts.end()) ? it->second : 0;
}

bool JITCompiler::isHotFunction(uint32_t functionIndex) const {
    auto it = callCounts.find(functionIndex);
    return it != callCounts.end() && it->second >= JIT_THRESHOLD;
}

std::function<void()>* JITCompiler::getCompiledFunction(uint32_t functionIndex) {
    auto it = compiledFunctions.find(functionIndex);
    if (it != compiledFunctions.end()) {
        return &it->second;
    }
    return nullptr;
}

std::optional<std::function<void()>> JITCompiler::compileFunction(uint32_t functionIndex) const {
    if (!vm || !vm->getChunk()) {
        return std::nullopt;
    }
    
    const auto& functions = vm->getChunk()->Functions();
    if (functionIndex >= functions.size()) {
        return std::nullopt;
    }
    
    const auto& func = functions[functionIndex];
    const auto& code = vm->getChunk()->Code();
    
    std::vector<std::function<void()>> instructions;
    
    size_t codeOffset = func.codeOffset;
    size_t endOffset = func.codeOffset + func.codeLength;
    
    while (codeOffset < endOffset && codeOffset < code.size()) {
        auto op = static_cast<bytecode::OpCode>(code[codeOffset]);
        codeOffset++;
        
        if (!generateInstruction(op, codeOffset, instructions)) {
            return std::nullopt;
        }
        
        if (op == bytecode::OpCode::RETURN || op == bytecode::OpCode::RETURN_VOID) {
            break;
        }
    }
    
    std::function<void()> compiledFunc;
    
    if (instructions.size() == 1) {
        compiledFunc = instructions[0];
    } else if (instructions.size() == 2) {
        auto& instr1 = instructions[0];
        auto& instr2 = instructions[1];
        compiledFunc = [instr1, instr2]() {
            instr1();
            instr2();
        };
    } else if (instructions.size() == 3) {
        auto& instr1 = instructions[0];
        auto& instr2 = instructions[1];
        auto& instr3 = instructions[2];
        compiledFunc = [instr1, instr2, instr3]() {
            instr1();
            instr2();
            instr3();
        };
    } else {
        if (instructions.size() <= 10) {
            const std::vector<std::function<void()>>& instrsCopy = instructions;
            compiledFunc = [instrsCopy]() {
                for (const auto& instr : instrsCopy) {
                    instr();
                }
            };
        } else {
            auto instrs = std::make_shared<std::vector<std::function<void()>>>(std::move(instructions));
            compiledFunc = [instrs]() {
                const auto& vec = *instrs;
                for (const auto & i : vec) {
                    i();
                }
            };
        }
    }
    
    return compiledFunc;
}

bool JITCompiler::generateInstruction(bytecode::OpCode op, size_t& codeOffset,
                                     std::vector<std::function<void()>>& code) const {
    VM* vmPtr = vm;
    auto* chunkPtr = vmPtr->getChunk();
    
    switch (op) {
        case bytecode::OpCode::PUSH_INT: {
            int64_t value = chunkPtr->ReadInt64(codeOffset);
            codeOffset += 8;
            code.emplace_back([vmPtr, value]() {
                if (vmPtr->stack.size() < VM::STACK_MAX) {
                    vmPtr->stack.emplace_back(value);
                }
            });
            break;
        }
        case bytecode::OpCode::PUSH_FLOAT: {
            double value = chunkPtr->ReadDouble(codeOffset);
            codeOffset += 8;
            code.emplace_back([vmPtr, value]() {
                if (vmPtr->stack.size() < VM::STACK_MAX) {
                    vmPtr->stack.emplace_back(value);
                }
            });
            break;
        }
        case bytecode::OpCode::PUSH_BOOL: {
            bool value = chunkPtr->ReadByte(codeOffset) != 0;
            codeOffset += 1;
            code.emplace_back([vmPtr, value]() {
                if (vmPtr->stack.size() < VM::STACK_MAX) {
                    vmPtr->stack.emplace_back(value);
                }
            });
            break;
        }
        case bytecode::OpCode::PUSH_CHAR: {
            char value = static_cast<char>(chunkPtr->ReadByte(codeOffset));
            codeOffset += 1;
            code.emplace_back([vmPtr, value]() {
                if (vmPtr->stack.size() < VM::STACK_MAX) {
                    vmPtr->stack.emplace_back(value);
                }
            });
            break;
        }
        case bytecode::OpCode::PUSH_NULL: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() < VM::STACK_MAX) {
                    vmPtr->stack.emplace_back();
                }
            });
            break;
        }
        case bytecode::OpCode::PUSH_STRING: {
            codeOffset += 4;
            code.emplace_back([vmPtr] {
                vmPtr->pushString();
            });
            break;
        }
        case bytecode::OpCode::PUSH_FUNC: {
            codeOffset += 4;
            code.emplace_back([vmPtr] {
                vmPtr->pushFunc();
            });
            break;
        }
        
        case bytecode::OpCode::LOAD_LOCAL: {
            uint32_t index = chunkPtr->ReadUint32(codeOffset);
            codeOffset += 4;
            code.emplace_back([vmPtr, index]() {
                if (!vmPtr->callStack.empty()) {
                    CallFrame& frame = vmPtr->callStack.top();
                    size_t stackIndex = frame.stackStart + index;
                    if (stackIndex < vmPtr->stack.size()) {
                        if (vmPtr->stack.size() < VM::STACK_MAX) {
                            vmPtr->stack.push_back(vmPtr->stack[stackIndex]);
                        }
                    }
                }
            });
            break;
        }
        case bytecode::OpCode::STORE_LOCAL: {
            uint32_t index = chunkPtr->ReadUint32(codeOffset);
            codeOffset += 4;
            code.emplace_back([vmPtr, index] {
                if (!vmPtr->callStack.empty() && !vmPtr->stack.empty()) {
                    const CallFrame& frame = vmPtr->callStack.top();
                    const size_t stackIndex = frame.stackStart + index;
                    const Value value = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    while (vmPtr->stack.size() <= stackIndex) {
                        vmPtr->stack.emplace_back();
                    }

                    vmPtr->stack[stackIndex] = value;
                }
            });
            break;
        }
        case bytecode::OpCode::LOAD_GLOBAL: {
            uint32_t index = chunkPtr->ReadUint32(codeOffset);
            codeOffset += 4;
            code.emplace_back([vmPtr, index]() {
                if (index < vmPtr->globals.size()) {
                    if (vmPtr->stack.size() < VM::STACK_MAX) {
                        vmPtr->stack.push_back(vmPtr->globals[index]);
                    }
                } else {
                    while (vmPtr->globals.size() <= index) {
                        vmPtr->globals.emplace_back();
                    }
                    if (vmPtr->stack.size() < VM::STACK_MAX) {
                        vmPtr->stack.push_back(vmPtr->globals[index]);
                    }
                }
            });
            break;
        }
        case bytecode::OpCode::STORE_GLOBAL: {
            uint32_t index = chunkPtr->ReadUint32(codeOffset);
            codeOffset += 4;
            code.emplace_back([vmPtr, index]() {
                if (!vmPtr->stack.empty()) {
                    while (vmPtr->globals.size() <= index) {
                        vmPtr->globals.emplace_back();
                    }
                    vmPtr->globals[index] = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                }
            });
            break;
        }
        
        case bytecode::OpCode::ADD: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
                        vmPtr->stack.emplace_back(a.asInt + b.asInt);
                    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
                        vmPtr->stack.emplace_back(a.asFloat + b.asFloat);
                    }
                }
            });
            break;
        }
        case bytecode::OpCode::SUB: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
                        vmPtr->stack.emplace_back(a.asInt - b.asInt);
                    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
                        vmPtr->stack.emplace_back(a.asFloat - b.asFloat);
                    }
                }
            });
            break;
        }
        case bytecode::OpCode::MUL: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
                        vmPtr->stack.emplace_back(a.asInt * b.asInt);
                    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
                        vmPtr->stack.emplace_back(a.asFloat * b.asFloat);
                    }
                }
            });
            break;
        }
        case bytecode::OpCode::DIV: {
            code.emplace_back([vmPtr]() {
                vmPtr->divide();
            });
            break;
        }
        case bytecode::OpCode::MOD: {
            code.emplace_back([vmPtr]() {
                vmPtr->modulo();
            });
            break;
        }
        case bytecode::OpCode::POW: {
            code.emplace_back([vmPtr]() {
                vmPtr->power();
            });
            break;
        }
        case bytecode::OpCode::NEG: {
            code.emplace_back([vmPtr]() {
                if (!vmPtr->stack.empty()) {
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    if (a.type == bytecode::ValueType::INT) {
                        vmPtr->stack.emplace_back(-a.asInt);
                    } else if (a.type == bytecode::ValueType::FLOAT) {
                        vmPtr->stack.emplace_back(-a.asFloat);
                    }
                }
            });
            break;
        }
        
        case bytecode::OpCode::AND: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    if (a.type == bytecode::ValueType::BOOL && b.type == bytecode::ValueType::BOOL) {
                        vmPtr->stack.emplace_back(a.asBool && b.asBool);
                    }
                }
            });
            break;
        }
        case bytecode::OpCode::OR: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    if (a.type == bytecode::ValueType::BOOL && b.type == bytecode::ValueType::BOOL) {
                        vmPtr->stack.emplace_back(a.asBool || b.asBool);
                    }
                }
            });
            break;
        }
        case bytecode::OpCode::NOT: {
            code.emplace_back([vmPtr]() {
                if (!vmPtr->stack.empty()) {
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    if (a.type == bytecode::ValueType::BOOL) {
                        vmPtr->stack.emplace_back(!a.asBool);
                    }
                }
            });
            break;
        }
        
        case bytecode::OpCode::CMP_EQ: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    bool result = false;
                    if (a.type == b.type) {
                        if (a.type == bytecode::ValueType::INT) {
                            result = a.asInt == b.asInt;
                        } else if (a.type == bytecode::ValueType::FLOAT) {
                            result = a.asFloat == b.asFloat;
                        } else if (a.type == bytecode::ValueType::BOOL) {
                            result = a.asBool == b.asBool;
                        } else if (a.type == bytecode::ValueType::CHAR) {
                            result = a.asChar == b.asChar;
                        }
                    }
                    vmPtr->stack.emplace_back(result);
                }
            });
            break;
        }
        case bytecode::OpCode::CMP_NE: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    bool result = true;
                    if (a.type == b.type) {
                        if (a.type == bytecode::ValueType::INT) {
                            result = a.asInt != b.asInt;
                        } else if (a.type == bytecode::ValueType::FLOAT) {
                            result = a.asFloat != b.asFloat;
                        } else if (a.type == bytecode::ValueType::BOOL) {
                            result = a.asBool != b.asBool;
                        } else if (a.type == bytecode::ValueType::CHAR) {
                            result = a.asChar != b.asChar;
                        }
                    }
                    vmPtr->stack.emplace_back(result);
                }
            });
            break;
        }
        case bytecode::OpCode::CMP_LT: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    bool result = false;
                    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
                        result = a.asInt < b.asInt;
                    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
                        result = a.asFloat < b.asFloat;
                    }
                    vmPtr->stack.emplace_back(result);
                }
            });
            break;
        }
        case bytecode::OpCode::CMP_GT: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    bool result = false;
                    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
                        result = a.asInt > b.asInt;
                    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
                        result = a.asFloat > b.asFloat;
                    }
                    vmPtr->stack.emplace_back(result);
                }
            });
            break;
        }
        case bytecode::OpCode::CMP_LE: {
            code.emplace_back([vmPtr] {
                if (vmPtr->stack.size() >= 2) {
                    const Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    const Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    bool result = false;
                    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
                        result = a.asInt <= b.asInt;
                    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
                        result = a.asFloat <= b.asFloat;
                    }
                    vmPtr->stack.emplace_back(result);
                }
            });
            break;
        }
        case bytecode::OpCode::CMP_GE: {
            code.emplace_back([vmPtr]() {
                if (vmPtr->stack.size() >= 2) {
                    const Value b = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    const Value a = vmPtr->stack.back();
                    vmPtr->stack.pop_back();
                    bool result = false;
                    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
                        result = a.asInt >= b.asInt;
                    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
                        result = a.asFloat >= b.asFloat;
                    }
                    vmPtr->stack.emplace_back(result);
                }
            });
            break;
        }
        
        case bytecode::OpCode::JMP:
        case bytecode::OpCode::JMP_IF_FALSE:
        case bytecode::OpCode::JMP_IF_TRUE: {
            codeOffset += 4;
            return false;
        }
        
        case bytecode::OpCode::CALL: {
            codeOffset += 4;
            codeOffset += 1;
            code.emplace_back([vmPtr] {
                vmPtr->call();
            });
            break;
        }
        case bytecode::OpCode::CALL_BUILTIN: {
            codeOffset += 1;
            codeOffset += 1;
            code.emplace_back([vmPtr] {
                vmPtr->callBuiltin();
            });
            break;
        }
        case bytecode::OpCode::RETURN: {
            code.emplace_back([vmPtr]() {
                vmPtr->returnOp();
            });
            break;
        }
        case bytecode::OpCode::RETURN_VOID: {
            code.emplace_back([vmPtr]() {
                vmPtr->returnVoid();
            });
            break;
        }
        
        case bytecode::OpCode::NEW_ARRAY: {
            codeOffset += 4;
            codeOffset += 1;
            code.emplace_back([vmPtr] {
                vmPtr->newArray();
            });
            break;
        }
        case bytecode::OpCode::ARRAY_GET: {
            codeOffset += 4;
            code.emplace_back([vmPtr] {
                vmPtr->arrayGet();
            });
            break;
        }
        case bytecode::OpCode::ARRAY_SET: {
            codeOffset += 4;
            code.emplace_back([vmPtr] {
                vmPtr->arraySet();
            });
            break;
        }
        case bytecode::OpCode::ARRAY_LEN: {
            code.emplace_back([vmPtr]() {
                vmPtr->arrayLen();
            });
            break;
        }
        
        case bytecode::OpCode::CAST_INT:
        case bytecode::OpCode::CAST_FLOAT:
        case bytecode::OpCode::CAST_BOOL:
        case bytecode::OpCode::CAST_CHAR:
        case bytecode::OpCode::CAST_STRING: {
            bytecode::OpCode capturedOp = op;
            code.emplace_back([vmPtr, capturedOp]() {
                vmPtr->executeInstruction(capturedOp);
            });
            break;
        }
        
        case bytecode::OpCode::POP: {
            code.emplace_back([vmPtr]() {
                if (!vmPtr->stack.empty()) {
                    vmPtr->stack.pop_back();
                }
            });
            break;
        }
        case bytecode::OpCode::DUP: {
            code.emplace_back([vmPtr]() {
                if (!vmPtr->stack.empty() && vmPtr->stack.size() < VM::STACK_MAX) {
                    vmPtr->stack.push_back(vmPtr->stack.back());
                }
            });
            break;
        }
        
        case bytecode::OpCode::NOP: {
            break;
        }
        case bytecode::OpCode::HALT: {
            code.emplace_back([vmPtr]() {
                vmPtr->halt();
            });
            break;
        }
        
        default:
            return false;
    }
    
    return true;
}

}
