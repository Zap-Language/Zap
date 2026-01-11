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
    const size_t funcStart = func.codeOffset;
    const size_t funcEnd = func.codeOffset + func.codeLength;

    JITCompiler* self = const_cast<JITCompiler*>(this);
    VM* vmPtr = vm;
    auto* chunkPtr = vm->getChunk();

    struct FoldAction {
        enum class Kind { None, PushInt, PushFloat, Skip } kind{Kind::None};
        int64_t intVal{};
        double floatVal{};
        uint32_t skipBytesAfterOp{0};
    };

    auto operandSize = [](bytecode::OpCode op) -> uint32_t {
        switch (op) {
            case bytecode::OpCode::PUSH_INT:    return 8;
            case bytecode::OpCode::PUSH_FLOAT:  return 8;
            case bytecode::OpCode::PUSH_BOOL:   return 1;
            case bytecode::OpCode::PUSH_CHAR:   return 1;
            case bytecode::OpCode::PUSH_STRING: return 4;
            case bytecode::OpCode::PUSH_FUNC:   return 4;
            case bytecode::OpCode::LOAD_LOCAL:  return 4;
            case bytecode::OpCode::STORE_LOCAL: return 4;
            case bytecode::OpCode::LOAD_GLOBAL: return 4;
            case bytecode::OpCode::STORE_GLOBAL:return 4;
            case bytecode::OpCode::JMP:         return 4;
            case bytecode::OpCode::JMP_IF_FALSE:return 4;
            case bytecode::OpCode::JMP_IF_TRUE: return 4;
            case bytecode::OpCode::CALL:        return 5;
            case bytecode::OpCode::CALL_BUILTIN:return 2;
            case bytecode::OpCode::NEW_ARRAY:   return 5;
            case bytecode::OpCode::ARRAY_GET:   return 4;
            case bytecode::OpCode::ARRAY_SET:   return 4;
            case bytecode::OpCode::CAST_INT:
            case bytecode::OpCode::CAST_FLOAT:
            case bytecode::OpCode::CAST_BOOL:
            case bytecode::OpCode::CAST_CHAR:
            case bytecode::OpCode::CAST_STRING:
            default:                            return 0;
        }
    };

    std::vector<FoldAction> foldTable(chunkPtr->Code().size());

    size_t scan = funcStart;
    while (scan < funcEnd && scan < chunkPtr->Code().size()) {
        const auto op1 = static_cast<bytecode::OpCode>(chunkPtr->Code()[scan]);
        if (op1 != bytecode::OpCode::PUSH_INT && op1 != bytecode::OpCode::PUSH_FLOAT) {
            scan += 1 + operandSize(op1);
            continue;
        }

        const size_t op1Size = 1 + operandSize(op1);
        const size_t next1 = scan + op1Size;
        if (next1 >= funcEnd || next1 >= chunkPtr->Code().size()) {
            break;
        }

        const auto op2 = static_cast<bytecode::OpCode>(chunkPtr->Code()[next1]);
        if (op2 != op1) {
            scan += 1 + operandSize(op1);
            continue;
        }

        const size_t op2Size = 1 + operandSize(op2);
        const size_t next2 = next1 + op2Size;
        if (next2 >= funcEnd || next2 >= chunkPtr->Code().size()) {
            break;
        }

        const auto op3 = static_cast<bytecode::OpCode>(chunkPtr->Code()[next2]);
        if (op3 != bytecode::OpCode::ADD && op3 != bytecode::OpCode::SUB && op3 != bytecode::OpCode::MUL) {
            scan += 1 + operandSize(op1);
            continue;
        }

        if (op1 == bytecode::OpCode::PUSH_INT) {
            int64_t a = chunkPtr->ReadInt64(scan + 1);
            int64_t b = chunkPtr->ReadInt64(next1 + 1);
            int64_t r = 0;
            switch (op3) {
                case bytecode::OpCode::ADD: r = a + b; break;
                case bytecode::OpCode::SUB: r = a - b; break;
                case bytecode::OpCode::MUL: r = a * b; break;
                default: break;
            }
            foldTable[scan] = {FoldAction::Kind::PushInt, r, 0.0, static_cast<uint32_t>(operandSize(op1))};
        } else {
            double a = chunkPtr->ReadDouble(scan + 1);
            double b = chunkPtr->ReadDouble(next1 + 1);
            double r = 0.0;
            switch (op3) {
                case bytecode::OpCode::ADD: r = a + b; break;
                case bytecode::OpCode::SUB: r = a - b; break;
                case bytecode::OpCode::MUL: r = a * b; break;
                default: break;
            }
            foldTable[scan] = {FoldAction::Kind::PushFloat, 0, r, static_cast<uint32_t>(operandSize(op1))};
        }

        foldTable[next1] = {FoldAction::Kind::Skip, 0, 0.0, static_cast<uint32_t>(operandSize(op2))};
        foldTable[next2] = {FoldAction::Kind::Skip, 0, 0.0, 0};

        scan = next2 + 1;
    }

    std::function<void()> compiledFunc = [vmPtr, funcStart, funcEnd, self, foldTable]() {
        auto* chunkPtr = vmPtr->getChunk();
        if (!chunkPtr) {
            vmPtr->status = VM::Status::RUNTIME_ERROR;
            return;
        }

        auto& code = chunkPtr->Code();

        vmPtr->ip = funcStart;

        while (vmPtr->status == VM::Status::OK && vmPtr->ip < funcEnd && vmPtr->ip < code.size()) {
            const size_t instrIndex = vmPtr->ip;
            const auto op = static_cast<bytecode::OpCode>(code[instrIndex]);
            vmPtr->ip++;
            const FoldAction action = foldTable[instrIndex];
            if (action.kind != FoldAction::Kind::None) {
                switch (action.kind) {
                    case FoldAction::Kind::PushInt:
                        vmPtr->push(Value(action.intVal));
                        vmPtr->ip += action.skipBytesAfterOp;
                        break;
                    case FoldAction::Kind::PushFloat:
                        vmPtr->push(Value(action.floatVal));
                        vmPtr->ip += action.skipBytesAfterOp;
                        break;
                    case FoldAction::Kind::Skip:
                        vmPtr->ip += action.skipBytesAfterOp;
                        break;
                    default:
                        break;
                }
                continue;
            }

            switch (op) {
                case bytecode::OpCode::PUSH_INT:      vmPtr->pushInt(); break;
                case bytecode::OpCode::PUSH_FLOAT:    vmPtr->pushFloat(); break;
                case bytecode::OpCode::PUSH_BOOL:     vmPtr->pushBool(); break;
                case bytecode::OpCode::PUSH_CHAR:     vmPtr->pushChar(); break;
                case bytecode::OpCode::PUSH_STRING:   vmPtr->pushString(); break;
                case bytecode::OpCode::PUSH_NULL:     vmPtr->pushNull(); break;
                case bytecode::OpCode::PUSH_FUNC:     vmPtr->pushFunc(); break;

                case bytecode::OpCode::LOAD_LOCAL:    vmPtr->loadLocal(); break;
                case bytecode::OpCode::STORE_LOCAL:   vmPtr->storeLocal(); break;
                case bytecode::OpCode::LOAD_GLOBAL:   vmPtr->loadGlobal(); break;
                case bytecode::OpCode::STORE_GLOBAL:  vmPtr->storeGlobal(); break;

                case bytecode::OpCode::ADD:           vmPtr->add(); break;
                case bytecode::OpCode::SUB:           vmPtr->subtract(); break;
                case bytecode::OpCode::MUL:           vmPtr->multiply(); break;
                case bytecode::OpCode::DIV:           vmPtr->divide(); break;
                case bytecode::OpCode::MOD:           vmPtr->modulo(); break;
                case bytecode::OpCode::POW:           vmPtr->power(); break;
                case bytecode::OpCode::NEG:           vmPtr->negate(); break;

                case bytecode::OpCode::AND:           vmPtr->andOp(); break;
                case bytecode::OpCode::OR:            vmPtr->orOp(); break;
                case bytecode::OpCode::NOT:           vmPtr->notOp(); break;

                case bytecode::OpCode::CMP_EQ:        vmPtr->cmpEq(); break;
                case bytecode::OpCode::CMP_NE:        vmPtr->cmpNe(); break;
                case bytecode::OpCode::CMP_LT:        vmPtr->cmpLt(); break;
                case bytecode::OpCode::CMP_GT:        vmPtr->cmpGt(); break;
                case bytecode::OpCode::CMP_LE:        vmPtr->cmpLe(); break;
                case bytecode::OpCode::CMP_GE:        vmPtr->cmpGe(); break;

                case bytecode::OpCode::JMP:           vmPtr->jump(); break;
                case bytecode::OpCode::JMP_IF_FALSE:  vmPtr->jumpIfFalse(); break;
                case bytecode::OpCode::JMP_IF_TRUE:   vmPtr->jumpIfTrue(); break;

                case bytecode::OpCode::CALL: {
                    const size_t frameDepthBefore = vmPtr->callStack.size();
                    vmPtr->call();

                    if (vmPtr->status != VM::Status::OK) {
                        break;
                    }
                    if (vmPtr->callStack.size() > frameDepthBefore) {
                        while (vmPtr->status == VM::Status::OK && vmPtr->callStack.size() > frameDepthBefore) {
                            if (vmPtr->ip >= code.size()) {
                                vmPtr->status = VM::Status::RUNTIME_ERROR;
                                break;
                            }
                            const auto innerOp = static_cast<bytecode::OpCode>(code[vmPtr->ip]);
                            vmPtr->ip++;
                            vmPtr->executeInstruction(innerOp);
                        }
                    }
                    break;
                }

                case bytecode::OpCode::CALL_BUILTIN:  vmPtr->callBuiltin(); break;

                case bytecode::OpCode::RETURN:        vmPtr->returnOp(); return;
                case bytecode::OpCode::RETURN_VOID:   vmPtr->returnVoid(); return;

                case bytecode::OpCode::NEW_ARRAY:     vmPtr->newArray(); break;
                case bytecode::OpCode::ARRAY_GET:     vmPtr->arrayGet(); break;
                case bytecode::OpCode::ARRAY_SET:     vmPtr->arraySet(); break;
                case bytecode::OpCode::ARRAY_LEN:     vmPtr->arrayLen(); break;

                case bytecode::OpCode::CAST_INT:
                case bytecode::OpCode::CAST_FLOAT:
                case bytecode::OpCode::CAST_BOOL:
                case bytecode::OpCode::CAST_CHAR:
                case bytecode::OpCode::CAST_STRING:
                    vmPtr->executeInstruction(op);
                    break;

                case bytecode::OpCode::POP:           vmPtr->popOp(); break;
                case bytecode::OpCode::DUP:           vmPtr->dup(); break;

                case bytecode::OpCode::NOP:           break;
                case bytecode::OpCode::HALT:          vmPtr->halt(); return;

                default:
                    vmPtr->status = VM::Status::RUNTIME_ERROR;
                    return;
            }
        }
    };

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
