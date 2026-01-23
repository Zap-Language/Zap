#pragma once

#include "obj.h"
#include "value.h"
#include <vector>

namespace jit {

struct ObjStruct : public Obj {
    std::vector<Value> fields;

    explicit ObjStruct(size_t fieldCount)
        : Obj(Type::STRUCT) {
        fields.resize(fieldCount);
    }

    ~ObjStruct() override = default;

    void markChildren() override {
        for (auto &v : fields) {
            if (v.isObj() && v.asObj) {
                v.asObj->marked = true;
                v.asObj->markChildren();
            }
        }
    }
};

}
