#pragma once

#include "obj.h"
#include "value.h"
#include <vector>

namespace jit {

struct ObjArray : public Obj {
    std::vector<Value> elements;

    ObjArray() : Obj(Type::ARRAY) {}
    
    explicit ObjArray(size_t size) : Obj(Type::ARRAY) {
        elements.resize(size);
    }

    ~ObjArray() override = default;

    void markChildren() override {
        for (auto& elem : elements) {
            if (elem.isObj() && elem.asObj) {
                elem.asObj->marked = true;
                elem.asObj->markChildren();
            }
        }
    }

    [[nodiscard]] size_t size() const { return elements.size(); }
    Value& operator[](size_t index) { return elements[index]; }
    const Value& operator[](size_t index) const { return elements[index]; }
};

}
