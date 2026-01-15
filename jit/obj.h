#pragma once

#include <cstdint>

namespace jit {

struct Obj {
    enum class Type : uint8_t {
        STRING,
        ARRAY,
        FUNCTION,
    };

    Type type;
    bool marked;
    Obj* next;

    Obj(Type t) : type(t), marked(false), next(nullptr) {}
    virtual ~Obj() = default;

    virtual void markChildren() {}
};

}
