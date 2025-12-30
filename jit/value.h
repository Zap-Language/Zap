#pragma once

#include "obj.h"
#include "../lib/bytecode/value_type.h"
#include <cstdint>
#include <cstring>

namespace jit {

struct Value {
    bytecode::ValueType type;
    
    union {
        int64_t asInt;
        double asFloat;
        bool asBool;
        char asChar;
        Obj* asObj;
    };

    Value() : type(bytecode::ValueType::VOID) {
        asInt = 0;
    }

    Value(int64_t val) : type(bytecode::ValueType::INT), asInt(val) {}
    Value(double val) : type(bytecode::ValueType::FLOAT), asFloat(val) {}
    Value(bool val) : type(bytecode::ValueType::BOOL), asBool(val) {}
    Value(char val) : type(bytecode::ValueType::CHAR), asChar(val) {}
    
    Value(Obj* obj) : type(bytecode::ValueType::STRING), asObj(obj) {
        if (obj) {
            switch (obj->type) {
                case Obj::Type::STRING:
                    type = bytecode::ValueType::STRING;
                    break;
                case Obj::Type::ARRAY:
                    type = bytecode::ValueType::ARRAY;
                    break;
                case Obj::Type::FUNCTION:
                    type = bytecode::ValueType::FUNCTION;
                    break;
            }
        }
    }

    bool isObj() const {
        return type == bytecode::ValueType::STRING ||
               type == bytecode::ValueType::ARRAY ||
               type == bytecode::ValueType::FUNCTION;
    }

    Obj* getObj() const {
        return isObj() ? asObj : nullptr;
    }
};

}
