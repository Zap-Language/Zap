#pragma once

#include <cstdint>
#include <string>

namespace bytecode {

    enum class ValueType : uint8_t {
        VOID = 0,
        INT = 1,
        FLOAT = 2,
        BOOL = 3,
        CHAR = 4,
        STRING = 5,
        ARRAY = 6,
        FUNCTION = 7,
    };

    inline std::string ValueTypeToString(ValueType type) {
        switch (type) {
        case ValueType::VOID:     return "void";
        case ValueType::INT:      return "int";
        case ValueType::FLOAT:    return "float";
        case ValueType::BOOL:     return "bool";
        case ValueType::CHAR:     return "char";
        case ValueType::STRING:   return "string";
        case ValueType::ARRAY:    return "array";
        case ValueType::FUNCTION: return "function";
        default:                  return "unknown";
        }
    }

}