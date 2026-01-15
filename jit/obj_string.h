#pragma once

#include <cstring>

#include "obj.h"
#include <string>

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

    explicit ObjString(const std::string& str)
        : ObjString(str.c_str(), str.length()) {}

    ~ObjString() override {
        delete[] chars;
    }

    [[nodiscard]] const char* c_str() const { return chars; }
    [[nodiscard]] std::string toString() const { return {chars, length}; }
};

}
