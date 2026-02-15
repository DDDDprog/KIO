/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include <vector>
#include <set>
#include "axeon/bytecode.hpp"

namespace kio {

class MemoryManager {
    std::vector<Obj*> objects;
    size_t bytesAllocated {0};
    size_t nextGC {1024 * 1024}; // 1MB threshold

public:
    void* allocate(size_t size, ObjType type) {
        bytesAllocated += size;
        if (bytesAllocated > nextGC) {
            collect();
        }
        
        Obj* obj = nullptr;
        if (type == ObjType::OBJ_ARRAY) obj = new ObjArray();
        else if (type == ObjType::OBJ_STRING) obj = new ObjString("");
        
        if (obj) objects.push_back(obj);
        return obj;
    }

    void collect() {
        std::set<Obj*> marked;
        for (auto it = objects.begin(); it != objects.end(); ) {
            if (marked.find(*it) == marked.end()) {
                delete *it;
                it = objects.erase(it);
            } else {
                ++it;
            }
        }
        nextGC = bytesAllocated * 2;
    }

    ~MemoryManager() {
        for (auto obj : objects) delete obj;
    }
};

static MemoryManager g_mem;

ObjArray* allocate_array() {
    return (ObjArray*)g_mem.allocate(sizeof(ObjArray), ObjType::OBJ_ARRAY);
}

} // namespace kio
