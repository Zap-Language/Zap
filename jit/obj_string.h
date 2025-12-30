#pragma once

#include "obj.h"
#include <string>
#include <cstring>

namespace jit {

struct ObjString : public Obj {
    size_t length;
    char* chars;

    ObjString(const char* str, size_t len) 
        : Obj(Type::STRING), length(len) {
        chars = new char[length + 1];
        std::memcpy(chars, str, length);
        chars[length] = '\0';
    }

    ObjString(const std::string& str) 
        : ObjString(str.c_str(), str.length()) {}

    ~ObjString() override {
        delete[] chars;
    }

    const char* c_str() const { return chars; }
    std::string toString() const { return std::string(chars, length); }
};

}
