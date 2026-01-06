#include "gc.h"
#include "obj_string.h"
#include "obj_array.h"
#include "obj_function.h"
#include <vector>

namespace jit {

GarbageCollector::GarbageCollector()
    : objects(nullptr)
    , bytesAllocated(0)
    , nextGC(INITIAL_GC_THRESHOLD)
    , objectCount(0) {
}

GarbageCollector::~GarbageCollector() {
    const Obj* current = objects;
    while (current != nullptr) {
        const Obj* next = current->next;
        delete current;
        current = next;
    }
}

void GarbageCollector::collect() {
    collectGarbage(roots);
}

void GarbageCollector::collectGarbage(const std::vector<Value*>& rootValues) {
    markReachable();
    sweep();
    nextGC = bytesAllocated * GC_GROWTH_FACTOR;
}

void GarbageCollector::markReachable() const {
    Obj* current = objects;
    while (current != nullptr) {
        current->marked = false;
        current = current->next;
    }

    std::vector<Obj*> workList;
    
    for (const Value* root : roots) {
        if (root && root->isObj() && root->asObj && !root->asObj->marked) {
            root->asObj->marked = true;
            workList.push_back(root->asObj);
        }
    }

    while (!workList.empty()) {
        Obj* obj = workList.back();
        workList.pop_back();

        if (obj->type == Obj::Type::ARRAY) {
            for (auto* arr = dynamic_cast<ObjArray*>(obj); auto& elem : arr->elements) {
                if (elem.isObj() && elem.asObj && !elem.asObj->marked) {
                    elem.asObj->marked = true;
                    workList.push_back(elem.asObj);
                }
            }
        }
    }
}

void GarbageCollector::sweep() {
    Obj** current = &objects;
    size_t freedBytes = 0;
    size_t freedObjects = 0;

    while (*current != nullptr) {
        if (!(*current)->marked) {
            Obj* unreached = *current;
            *current = unreached->next;

            size_t objSize = calculateObjectSize(unreached);

            delete unreached;
            bytesAllocated -= objSize;
            objectCount--;
            freedBytes += objSize;
            freedObjects++;
        } else {
            current = &((*current)->next);
        }
    }
}

size_t GarbageCollector::calculateObjectSize(Obj* obj) {
    switch (obj->type) {
        case Obj::Type::STRING: {
            auto* str = dynamic_cast<ObjString*>(obj);
            return sizeof(ObjString) + str->length + 1;
        }
        case Obj::Type::ARRAY: {
            auto* arr = dynamic_cast<ObjArray*>(obj);
            return sizeof(ObjArray) + arr->elements.size() * sizeof(Value);
        }
        case Obj::Type::FUNCTION: {
            return sizeof(ObjFunction);
        }
        default:
            return sizeof(Obj);
    }
}

}
