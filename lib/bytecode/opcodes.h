#pragma once

#include <cstdint>
#include <string>

namespace bytecode {

enum class OpCode : uint8_t {
   
    PUSH_INT,
    PUSH_FLOAT,
    PUSH_BOOL,
    PUSH_CHAR,
    PUSH_STRING,
    PUSH_NULL,
    PUSH_FUNC,

   
    LOAD_LOCAL,
    STORE_LOCAL,
    LOAD_GLOBAL,
    STORE_GLOBAL,

    JMP,
    JMP_IF_FALSE,
    JMP_IF_TRUE,

    CALL,
    CALL_VALUE,
    CALL_BUILTIN,

    RETURN,
    RETURN_VOID,

    NEW_ARRAY,
    ARRAY_GET,
    ARRAY_SET,
    ARRAY_LEN,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    NEG,

   
    AND,
    OR,
    NOT,

   
    CMP_EQ,
    CMP_NE,
    CMP_LT,
    CMP_GT,
    CMP_LE,
    CMP_GE,

    NEW_STRUCT,
    GET_FIELD,
    SET_FIELD,

    CAST_INT,
    CAST_FLOAT,
    CAST_BOOL,
    CAST_CHAR,
    CAST_STRING,

   
    POP,
    DUP,

   
    NOP,
    HALT,
};

enum class BuiltinFunction : uint8_t {
    PRINT = 0,
    LEN = 1,
    READ = 2,
    CAST_INT = 3,
    CAST_FLOAT = 4,
    CAST_BOOL = 5,
    CAST_CHAR = 6,
    CAST_STRING = 7,
};

inline size_t GetOperandSize(OpCode op) {
    switch (op) {
        case OpCode::PUSH_INT:
        case OpCode::PUSH_FLOAT:
            return 8;
        case OpCode::PUSH_BOOL:
        case OpCode::PUSH_CHAR:
            return 1;
        case OpCode::PUSH_STRING:
        case OpCode::PUSH_FUNC:
        case OpCode::LOAD_LOCAL:
        case OpCode::STORE_LOCAL:
        case OpCode::LOAD_GLOBAL:
        case OpCode::STORE_GLOBAL:
        case OpCode::JMP:
        case OpCode::JMP_IF_FALSE:
        case OpCode::JMP_IF_TRUE:
            return 4;
        case OpCode::CALL:
            return 5;
        case OpCode::CALL_VALUE:
            return 1;
        case OpCode::CALL_BUILTIN:
            return 2;
        case OpCode::NEW_ARRAY:
            return 5;
        case OpCode::NEW_STRUCT:
            return 5;
        case OpCode::GET_FIELD:
        case OpCode::SET_FIELD:
            return 5;
        default:
            return 0;
    }
}

inline std::string OpCodeToString(OpCode op) {
    switch (op) {
        case OpCode::PUSH_INT:      return "PUSH_INT";
        case OpCode::PUSH_FLOAT:    return "PUSH_FLOAT";
        case OpCode::PUSH_BOOL:     return "PUSH_BOOL";
        case OpCode::PUSH_CHAR:     return "PUSH_CHAR";
        case OpCode::PUSH_STRING:   return "PUSH_STRING";
        case OpCode::PUSH_NULL:     return "PUSH_NULL";
        case OpCode::PUSH_FUNC:     return "PUSH_FUNC";
        case OpCode::LOAD_LOCAL:    return "LOAD_LOCAL";
        case OpCode::STORE_LOCAL:   return "STORE_LOCAL";
        case OpCode::LOAD_GLOBAL:   return "LOAD_GLOBAL";
        case OpCode::STORE_GLOBAL:  return "STORE_GLOBAL";
        case OpCode::JMP:           return "JMP";
        case OpCode::JMP_IF_FALSE:  return "JMP_IF_FALSE";
        case OpCode::JMP_IF_TRUE:   return "JMP_IF_TRUE";
        case OpCode::CALL:          return "CALL";
        case OpCode::CALL_VALUE:    return "CALL_VALUE";
        case OpCode::CALL_BUILTIN:  return "CALL_BUILTIN";
        case OpCode::RETURN:        return "RETURN";
        case OpCode::RETURN_VOID:   return "RETURN_VOID";
        case OpCode::NEW_ARRAY:     return "NEW_ARRAY";
        case OpCode::ARRAY_GET:     return "ARRAY_GET";
        case OpCode::ARRAY_SET:     return "ARRAY_SET";
        case OpCode::ARRAY_LEN:     return "ARRAY_LEN";
        case OpCode::ADD:           return "ADD";
        case OpCode::SUB:           return "SUB";
        case OpCode::MUL:           return "MUL";
        case OpCode::DIV:           return "DIV";
        case OpCode::MOD:           return "MOD";
        case OpCode::POW:           return "POW";
        case OpCode::NEG:           return "NEG";
        case OpCode::AND:           return "AND";
        case OpCode::OR:            return "OR";
        case OpCode::NOT:           return "NOT";
        case OpCode::CMP_EQ:        return "CMP_EQ";
        case OpCode::CMP_NE:        return "CMP_NE";
        case OpCode::CMP_LT:        return "CMP_LT";
        case OpCode::CMP_GT:        return "CMP_GT";
        case OpCode::CMP_LE:        return "CMP_LE";
        case OpCode::CMP_GE:        return "CMP_GE";
        case OpCode::NEW_STRUCT:    return "NEW_STRUCT";
        case OpCode::GET_FIELD:     return "GET_FIELD";
        case OpCode::SET_FIELD:     return "SET_FIELD";
        case OpCode::CAST_INT:      return "CAST_INT";
        case OpCode::CAST_FLOAT:    return "CAST_FLOAT";
        case OpCode::CAST_BOOL:     return "CAST_BOOL";
        case OpCode::CAST_CHAR:     return "CAST_CHAR";
        case OpCode::CAST_STRING:   return "CAST_STRING";
        case OpCode::POP:           return "POP";
        case OpCode::DUP:           return "DUP";
        case OpCode::NOP:           return "NOP";
        case OpCode::HALT:          return "HALT";
        default:                    return "UNKNOWN";
    }
}

}