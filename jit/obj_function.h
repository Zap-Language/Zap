#pragma once

#include "obj.h"
#include "../lib/bytecode/bytecode_chunk.h"
#include <vector>

namespace jit {

struct ObjFunction : public Obj {
    uint32_t functionIndex;

    ObjFunction(uint32_t index) 
        : Obj(Type::FUNCTION), functionIndex(index) {}

    ~ObjFunction() override = default;
};

}
