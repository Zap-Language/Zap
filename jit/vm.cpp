#include "vm.h"
#include "jit.h"
#include "obj_string.h"
#include "obj_array.h"
#include "obj_function.h"
#include <iostream>
#include <cmath>
#include <memory>

namespace jit {

VM::VM() 
    : ip(0)
    , jitCompiler(nullptr)
    , status(Status::OK) {
}

VM::~VM() = default;

void VM::loadChunk(std::unique_ptr<bytecode::BytecodeChunk> newChunk) {
    chunk = std::move(newChunk);
    ip = 0;
    stack.clear();
    globals.clear();
    callStack = std::stack<CallFrame>();
    status = Status::OK;
}

void VM::run() {
    if (!chunk) {
        status = Status::RUNTIME_ERROR;
        return;
    }

    const auto& code = chunk->Code();

    while (status == Status::OK) {
        if (ip >= code.size()) {
            break;
        }
        
        registerRoots();

        const auto op = static_cast<bytecode::OpCode>(code[ip]);
        ip++;

        executeInstruction(op);
    }
}

void VM::registerRoots() {
    std::vector<Value*> roots;

    for (auto& val : stack) {
        if (val.isObj()) {
            roots.push_back(&val);
        }
    }

    for (auto& val : globals) {
        if (val.isObj()) {
            roots.push_back(&val);
        }
    }

    gc.setRoots(roots);
}

void VM::push(const Value& value) {
    if (stack.size() >= STACK_MAX) {
        status = Status::STACK_OVERFLOW;
        return;
    }
    stack.push_back(value);
}

Value VM::pop() {
    if (stack.empty()) {
        status = Status::STACK_UNDERFLOW;
        return {};
    }
    const Value value = stack.back();
    stack.pop_back();
    return value;
}

Value VM::peek(size_t distance) const {
    if (distance >= stack.size()) {
        return {};
    }
    return stack[stack.size() - 1 - distance];
}

void VM::executeInstruction(bytecode::OpCode op) {
    switch (op) {
        case bytecode::OpCode::PUSH_INT:      pushInt(); break;
        case bytecode::OpCode::PUSH_FLOAT:    pushFloat(); break;
        case bytecode::OpCode::PUSH_BOOL:      pushBool(); break;
        case bytecode::OpCode::PUSH_CHAR:      pushChar(); break;
        case bytecode::OpCode::PUSH_STRING:    pushString(); break;
        case bytecode::OpCode::PUSH_NULL:      pushNull(); break;
        case bytecode::OpCode::PUSH_FUNC:      pushFunc(); break;

        case bytecode::OpCode::LOAD_LOCAL:     loadLocal(); break;
        case bytecode::OpCode::STORE_LOCAL:    storeLocal(); break;
        case bytecode::OpCode::LOAD_GLOBAL:    loadGlobal(); break;
        case bytecode::OpCode::STORE_GLOBAL:   storeGlobal(); break;

        case bytecode::OpCode::ADD:            add(); break;
        case bytecode::OpCode::SUB:            subtract(); break;
        case bytecode::OpCode::MUL:            multiply(); break;
        case bytecode::OpCode::DIV:            divide(); break;
        case bytecode::OpCode::MOD:            modulo(); break;
        case bytecode::OpCode::POW:            power(); break;
        case bytecode::OpCode::NEG:            negate(); break;

        case bytecode::OpCode::AND:            andOp(); break;
        case bytecode::OpCode::OR:             orOp(); break;
        case bytecode::OpCode::NOT:            notOp(); break;

        case bytecode::OpCode::CMP_EQ:         cmpEq(); break;
        case bytecode::OpCode::CMP_NE:         cmpNe(); break;
        case bytecode::OpCode::CMP_LT:         cmpLt(); break;
        case bytecode::OpCode::CMP_GT:         cmpGt(); break;
        case bytecode::OpCode::CMP_LE:         cmpLe(); break;
        case bytecode::OpCode::CMP_GE:         cmpGe(); break;

        case bytecode::OpCode::JMP:            jump(); break;
        case bytecode::OpCode::JMP_IF_FALSE:   jumpIfFalse(); break;
        case bytecode::OpCode::JMP_IF_TRUE:    jumpIfTrue(); break;

        case bytecode::OpCode::CALL:           call(); break;
        case bytecode::OpCode::CALL_BUILTIN:   callBuiltin(); break;
        case bytecode::OpCode::RETURN:         returnOp(); break;
        case bytecode::OpCode::RETURN_VOID:    returnVoid(); break;

        case bytecode::OpCode::NEW_ARRAY:      newArray(); break;
        case bytecode::OpCode::ARRAY_GET:      arrayGet(); break;
        case bytecode::OpCode::ARRAY_SET:      arraySet(); break;
        case bytecode::OpCode::ARRAY_LEN:      arrayLen(); break;

        case bytecode::OpCode::POP:            popOp(); break;
        case bytecode::OpCode::DUP:            dup(); break;

        case bytecode::OpCode::NOP:            break;
        case bytecode::OpCode::HALT:           halt(); break;

        default:
            status = Status::RUNTIME_ERROR;
            break;
    }
}

void VM::pushInt() {
    int64_t value = chunk->ReadInt64(ip);
    ip += 8;
    push(Value(value));
}

void VM::pushFloat() {
    double value = chunk->ReadDouble(ip);
    ip += 8;
    push(Value(value));
}

void VM::pushBool() {
    bool value = chunk->ReadByte(ip) != 0;
    ip += 1;
    push(Value(value));
}

void VM::pushChar() {
    char value = static_cast<char>(chunk->ReadByte(ip));
    ip += 1;
    push(Value(value));
}

void VM::pushString() {
    uint32_t stringIndex = chunk->ReadUint32(ip);
    ip += 4;
    
    const auto& strings = chunk->Strings();
    if (stringIndex < strings.size()) {
        const std::string& str = strings[stringIndex];
        auto* objStr = gc.allocate<ObjString>(str);
        push(Value(objStr));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::pushNull() {
    push(Value());
}

void VM::pushFunc() {
    uint32_t funcIndex = chunk->ReadUint32(ip);
    ip += 4;
    
    auto* objFunc = gc.allocate<ObjFunction>(funcIndex);
    push(Value(objFunc));
}

void VM::loadLocal() {
    uint32_t index = chunk->ReadUint32(ip);
    ip += 4;
    
    if (!callStack.empty()) {
        CallFrame& frame = callStack.top();
        size_t stackIndex = frame.stackStart + index;
        if (stackIndex < stack.size()) {
            push(stack[stackIndex]);
        } else {
            status = Status::RUNTIME_ERROR;
        }
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::storeLocal() {
    uint32_t index = chunk->ReadUint32(ip);
    ip += 4;
    
    if (!callStack.empty()) {
        CallFrame& frame = callStack.top();
        size_t stackIndex = frame.stackStart + index;

        Value value = pop();

        while (stack.size() <= stackIndex) {
            stack.emplace_back();
        }
        
        stack[stackIndex] = value;
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::loadGlobal() {
    uint32_t index = chunk->ReadUint32(ip);
    ip += 4;
    
    if (index < globals.size()) {
        push(globals[index]);
    } else {
        while (globals.size() <= index) {
            globals.emplace_back();
        }
        push(globals[index]);
    }
}

void VM::storeGlobal() {
    uint32_t index = chunk->ReadUint32(ip);
    ip += 4;
    
    Value value = pop();
    
    while (globals.size() <= index) {
        globals.emplace_back();
    }
    
    globals[index] = value;
}

void VM::add() {
    Value b = pop();
    Value a = pop();
    
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        push(Value(a.asInt + b.asInt));
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        push(Value(a.asFloat + b.asFloat));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::subtract() {
    Value b = pop();
    Value a = pop();
    
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        push(Value(a.asInt - b.asInt));
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        push(Value(a.asFloat - b.asFloat));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::multiply() {
    Value b = pop();
    Value a = pop();
    
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        push(Value(a.asInt * b.asInt));
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        push(Value(a.asFloat * b.asFloat));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::divide() {
    Value b = pop();
    Value a = pop();
    
    if (b.type == bytecode::ValueType::INT && b.asInt == 0) {
        status = Status::RUNTIME_ERROR;
        return;
    }
    if (b.type == bytecode::ValueType::FLOAT && b.asFloat == 0.0) {
        status = Status::RUNTIME_ERROR;
        return;
    }
    
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        push(Value(a.asInt / b.asInt));
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        push(Value(a.asFloat / b.asFloat));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::modulo() {
    Value b = pop();
    Value a = pop();
    
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        if (b.asInt == 0) {
            status = Status::RUNTIME_ERROR;
            return;
        }
        push(Value(a.asInt % b.asInt));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::power() {
    Value b = pop();
    Value a = pop();
    
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        int64_t result = 1;
        for (int64_t i = 0; i < b.asInt; i++) {
            result *= a.asInt;
        }
        push(Value(result));
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        double result = std::pow(a.asFloat, b.asFloat);
        push(Value(result));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::negate() {
    Value a = pop();
    
    if (a.type == bytecode::ValueType::INT) {
        push(Value(-a.asInt));
    } else if (a.type == bytecode::ValueType::FLOAT) {
        push(Value(-a.asFloat));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::andOp() {
    Value b = pop();
    Value a = pop();
    
    if (a.type == bytecode::ValueType::BOOL && b.type == bytecode::ValueType::BOOL) {
        push(Value(a.asBool && b.asBool));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::orOp() {
    Value b = pop();
    Value a = pop();
    
    if (a.type == bytecode::ValueType::BOOL && b.type == bytecode::ValueType::BOOL) {
        push(Value(a.asBool || b.asBool));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::notOp() {
    Value a = pop();
    
    if (a.type == bytecode::ValueType::BOOL) {
        push(Value(!a.asBool));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::cmpEq() {
    Value b = pop();
    Value a = pop();
    
    bool result = false;
    if (a.type == b.type) {
        switch (a.type) {
            case bytecode::ValueType::INT:
                result = a.asInt == b.asInt;
                break;
            case bytecode::ValueType::FLOAT:
                result = a.asFloat == b.asFloat;
                break;
            case bytecode::ValueType::BOOL:
                result = a.asBool == b.asBool;
                break;
            case bytecode::ValueType::CHAR:
                result = a.asChar == b.asChar;
                break;
            case bytecode::ValueType::STRING:
                if (a.asObj && b.asObj && a.asObj->type == Obj::Type::STRING) {
                    auto strA = dynamic_cast<ObjString*>(a.asObj);
                    auto strB = dynamic_cast<ObjString*>(b.asObj);
                    result = strA->toString() == strB->toString();
                }
                break;
            default:
                result = false;
        }
    }
    push(Value(result));
}

void VM::cmpNe() {
    cmpEq();
    Value result = pop();
    push(Value(!result.asBool));
}

void VM::cmpLt() {
    Value b = pop();
    Value a = pop();
    
    bool result = false;
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        result = a.asInt < b.asInt;
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        result = a.asFloat < b.asFloat;
    }
    push(Value(result));
}

void VM::cmpGt() {
    Value b = pop();
    Value a = pop();
    
    bool result = false;
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        result = a.asInt > b.asInt;
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        result = a.asFloat > b.asFloat;
    }
    push(Value(result));
}

void VM::cmpLe() {
    Value b = pop();
    Value a = pop();
    
    bool result = false;
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        result = a.asInt <= b.asInt;
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        result = a.asFloat <= b.asFloat;
    }
    push(Value(result));
}

void VM::cmpGe() {
    Value b = pop();
    Value a = pop();
    
    bool result = false;
    if (a.type == bytecode::ValueType::INT && b.type == bytecode::ValueType::INT) {
        result = a.asInt >= b.asInt;
    } else if (a.type == bytecode::ValueType::FLOAT && b.type == bytecode::ValueType::FLOAT) {
        result = a.asFloat >= b.asFloat;
    }
    push(Value(result));
}

void VM::jump() {
    auto offset = static_cast<int32_t>(chunk->ReadUint32(ip));
    ip += 4;
    ip = static_cast<size_t>(offset);
}

void VM::jumpIfFalse() {
    auto offset = static_cast<int32_t>(chunk->ReadUint32(ip));
    ip += 4;
    
    Value condition = peek();
    if (condition.type == bytecode::ValueType::BOOL && !condition.asBool) {
        ip = static_cast<size_t>(offset);
    }
}

void VM::jumpIfTrue() {
    auto offset = static_cast<int32_t>(chunk->ReadUint32(ip));
    ip += 4;
    
    Value condition = peek();
    if (condition.type == bytecode::ValueType::BOOL && condition.asBool) {
        ip = static_cast<size_t>(offset);
    }
}

void VM::call() {
    uint32_t functionIndex = chunk->ReadUint32(ip);
    ip += 4;
    uint8_t argCount = chunk->ReadByte(ip);
    ip += 1;
    
    const auto& functions = chunk->Functions();
    if (functionIndex >= functions.size()) {
        status = Status::RUNTIME_ERROR;
        return;
    }
    
    const auto& func = functions[functionIndex];
    
    if (argCount != func.paramCount) {
        status = Status::RUNTIME_ERROR;
        return;
    }
    
    CallFrame frame{};
    frame.functionIndex = functionIndex;
    frame.ip = ip;
    frame.stackStart = stack.size() - argCount;
    frame.localCount = func.localCount;
    
    callStack.push(frame);
    
    if (jitCompiler) {
        jitCompiler->incrementCallCount(functionIndex);

        if (auto* compiledFunc = jitCompiler->getCompiledFunction(functionIndex)) {
            ip = func.codeOffset;
            (*compiledFunc)();
            return;
        }
    }
    
    ip = func.codeOffset;
}

void VM::callBuiltin() {
    uint8_t builtinIndex = chunk->ReadByte(ip);
    ip += 1;
    ip += 1;
    
    switch (static_cast<bytecode::BuiltinFunction>(builtinIndex)) {
        case bytecode::BuiltinFunction::PRINT:
            builtinPrint();
            break;
        case bytecode::BuiltinFunction::LEN:
            builtinLen();
            break;
        case bytecode::BuiltinFunction::READ:
            builtinRead();
            break;
        case bytecode::BuiltinFunction::CAST_INT:
            builtinCastInt();
            break;
        default:
            status = Status::RUNTIME_ERROR;
            break;
    }
}

void VM::returnOp() {
    if (!callStack.empty()) {
        Value returnValue = pop();
        
        CallFrame frame = callStack.top();
        callStack.pop();
        
        while (stack.size() > frame.stackStart) {
            stack.pop_back();
        }
        
        ip = frame.ip;
        
        push(returnValue);
    } else {
        status = Status::OK;
        ip = chunk->Code().size();
    }
}

void VM::returnVoid() {
    if (!callStack.empty()) {
        CallFrame frame = callStack.top();
        callStack.pop();

        while (stack.size() > frame.stackStart) {
            stack.pop_back();
        }

        ip = frame.ip;
    }
}

void VM::newArray() {
    ip += 1;
    uint32_t size = chunk->ReadUint32(ip);
    ip += 4;
    
    auto* arr = gc.allocate<ObjArray>(size);
    push(Value(arr));
}

void VM::arrayGet() {
    Value indexVal = pop();
    Value arrayVal = pop();

    if (arrayVal.type == bytecode::ValueType::ARRAY && arrayVal.asObj &&
        indexVal.type == bytecode::ValueType::INT) {
        auto* arr = dynamic_cast<ObjArray*>(arrayVal.asObj);
        const auto idx = static_cast<size_t>(indexVal.asInt);
        if (idx < arr->size()) {
            push(arr->elements[idx]);
        } else {
            status = Status::RUNTIME_ERROR;
        }
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::arraySet() {
    Value value = pop();
    Value indexVal = pop();
    Value arrayVal = pop();
    
    if (arrayVal.type == bytecode::ValueType::ARRAY && arrayVal.asObj &&
        indexVal.type == bytecode::ValueType::INT) {
        auto* arr = dynamic_cast<ObjArray*>(arrayVal.asObj);
        const auto idx = static_cast<size_t>(indexVal.asInt);
        if (idx < arr->size()) {
            arr->elements[idx] = value;
        } else {
            status = Status::RUNTIME_ERROR;
        }
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::arrayLen() {
    Value arrayVal = pop();
    if (arrayVal.type == bytecode::ValueType::ARRAY && arrayVal.asObj) {
        auto* arr = dynamic_cast<ObjArray*>(arrayVal.asObj);
        push(Value(static_cast<int64_t>(arr->size())));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::popOp() {
    pop();
}

void VM::dup() {
    Value top = peek();
    push(top);
}

void VM::halt() {
    status = Status::OK;
    ip = chunk->Code().size();
}

void VM::builtinCastInt() {
    if (stack.empty()) {
        std::cerr << "Error: Stack is empty when calling print" << std::endl;
        status = Status::STACK_UNDERFLOW;
        return;
    }

    Value value = pop();

    switch (value.type) {
        case bytecode::ValueType::STRING: {
            auto strObj = dynamic_cast<ObjString*>(value.asObj);
            push(Value(std::strtoll(strObj->c_str(), nullptr, 10)));
            break;
        }
        default:
            status = Status::RUNTIME_ERROR;
    }
}

void VM::builtinPrint() {
    if (stack.empty()) {
        std::cerr << "Error: Stack is empty when calling print" << std::endl;
        status = Status::STACK_UNDERFLOW;
        return;
    }
    
    Value value = pop();
    
    switch (value.type) {
        case bytecode::ValueType::INT:
            std::cout << value.asInt;
            break;
        case bytecode::ValueType::FLOAT:
            std::cout << value.asFloat;
            break;
        case bytecode::ValueType::BOOL:
            std::cout << (value.asBool ? "true" : "false");
            break;
        case bytecode::ValueType::CHAR:
            std::cout << value.asChar;
            break;
        case bytecode::ValueType::STRING:
            if (value.asObj && value.asObj->type == Obj::Type::STRING) {
                auto* str = dynamic_cast<ObjString*>(value.asObj);
                std::cout << str->toString();
            }
            break;
        case bytecode::ValueType::ARRAY:
            if (value.asObj && value.asObj->type == Obj::Type::ARRAY) {
                auto* arr = dynamic_cast<ObjArray*>(value.asObj);
                std::cout << "[";
                for (size_t i = 0; i < arr->elements.size(); ++i) {
                    const auto& el = arr->elements[i];
                    switch (el.type) {
                        case bytecode::ValueType::INT:   std::cout << el.asInt; break;
                        case bytecode::ValueType::FLOAT: std::cout << el.asFloat; break;
                        case bytecode::ValueType::BOOL:  std::cout << (el.asBool ? "true" : "false"); break;
                        case bytecode::ValueType::CHAR:  std::cout << el.asChar; break;
                        case bytecode::ValueType::STRING:
                            if (el.asObj && el.asObj->type == Obj::Type::STRING) {
                                std::cout << dynamic_cast<ObjString*>(el.asObj)->toString();
                            }
                            break;
                        case bytecode::ValueType::ARRAY:
                            std::cout << "<array>";
                            break;
                        default:
                            std::cout << "<unknown>";
                            break;
                    }
                    if (i + 1 < arr->elements.size()) {
                        std::cout << ", ";
                    }
                }
                std::cout << "]";
            }
            break;
        default:
            std::cerr << "Error: Unknown value type in print: " << static_cast<int>(value.type) << std::endl;
            break;
    }
    std::cout << std::endl;
    
    push(Value());
}

void VM::builtinLen() {
    Value value = pop();
    
    if (value.type == bytecode::ValueType::ARRAY && value.asObj) {
        auto* arr = dynamic_cast<ObjArray*>(value.asObj);
        push(Value(static_cast<int64_t>(arr->size())));
    } else {
        status = Status::RUNTIME_ERROR;
    }
}

void VM::builtinRead() {
    std::string input;
    std::getline(std::cin, input);
    auto* str = gc.allocate<ObjString>(input);
    push(Value(str));
}

}
