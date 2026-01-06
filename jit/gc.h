#pragma once

#include "obj.h"
#include "value.h"
#include <vector>

namespace jit {

class GarbageCollector {
public:
    GarbageCollector();
    ~GarbageCollector();

    template<typename T, typename... Args>
    T* allocate(Args&&... args) {
        if (shouldCollect()) {
            collectGarbage(roots);
        }

        T* obj = new T(std::forward<Args>(args)...);
        
        obj->next = objects;
        objects = obj;

        const size_t objSize = calculateObjectSize(obj);
        bytesAllocated += objSize;
        objectCount++;
        
        return obj;
    }

    void setRoots(const std::vector<Value*>& newRoots) {
        roots = newRoots;
    }

    void addRoot(Value* root) {
        roots.push_back(root);
    }

    void clearRoots() {
        roots.clear();
    }

    void collect();

    [[nodiscard]] size_t getBytesAllocated() const { return bytesAllocated; }
    [[nodiscard]] size_t getObjectCount() const { return objectCount; }
    [[nodiscard]] size_t getNextGC() const { return nextGC; }

private:
    Obj* objects;

    std::vector<Value*> roots;

    size_t bytesAllocated;
    size_t nextGC;
    size_t objectCount;

    static constexpr size_t INITIAL_GC_THRESHOLD = 1024 * 1024;
    static constexpr size_t GC_GROWTH_FACTOR = 2;

    [[nodiscard]] bool shouldCollect() const {
        return bytesAllocated >= nextGC;
    }

    void markReachable() const;

    void sweep();

    void collectGarbage(const std::vector<Value*>& rootValues);

    static size_t calculateObjectSize(Obj* obj);
};

}

