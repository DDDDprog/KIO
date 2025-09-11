#include "kio/memory_manager.hpp"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace kio {

struct MemoryManager::MemoryPool {
    void* data;
    size_t size;
    size_t used;
    
    MemoryPool(size_t pool_size) : size(pool_size), used(0) {
        data = std::malloc(pool_size);
    }
    
    ~MemoryPool() {
        if (data) {
            std::free(data);
        }
    }
};

MemoryManager::MemoryManager(size_t initial_pool_size) {
    pools_.emplace_back(std::make_unique<MemoryPool>(initial_pool_size));
}

MemoryManager::~MemoryManager() {
}

void* MemoryManager::allocate(size_t size, size_t alignment) {
    std::lock_guard<std::mutex> lock(allocation_mutex_);
    
    // Simple allocation from first available pool
    for (auto& pool : pools_) {
        if (pool->used + size <= pool->size) {
            void* ptr = static_cast<char*>(pool->data) + pool->used;
            pool->used += size;
            total_allocated_ += size;
            return ptr;
        }
    }
    
    // Need to expand pools
    expandPool();
    return allocate(size, alignment);
}

void MemoryManager::deallocate(void* ptr) {
    if (ptr) {
        // For now, just track that memory was deallocated
        // Real implementation would need to track individual allocations
    }
}

void MemoryManager::collectGarbage() {
    std::lock_guard<std::mutex> lock(allocation_mutex_);
    
    if (total_allocated_ > gc_threshold_) {
        compactPools();
    }
}

void MemoryManager::setGCThreshold(size_t threshold) {
    gc_threshold_ = threshold;
}

size_t MemoryManager::getTotalAllocated() const {
    return total_allocated_;
}

size_t MemoryManager::getPoolSize() const {
    size_t total = 0;
    for (const auto& pool : pools_) {
        total += pool->size;
    }
    return total;
}

size_t MemoryManager::getFragmentation() const {
    size_t total_size = getPoolSize();
    size_t used = total_allocated_;
    return total_size > 0 ? ((total_size - used) * 100) / total_size : 0;
}

void MemoryManager::setMemoryLimit(size_t limit) {
    memory_limit_ = limit;
}

bool MemoryManager::isMemoryLimitExceeded() const {
    return total_allocated_ > memory_limit_;
}

void MemoryManager::expandPool() {
    size_t new_size = pools_.empty() ? 1024 * 1024 : pools_.back()->size * 2;
    pools_.emplace_back(std::make_unique<MemoryPool>(new_size));
}

void MemoryManager::compactPools() {
    // Simple compaction - reset usage counters
    for (auto& pool : pools_) {
        pool->used = 0;
    }
    total_allocated_ = 0;
}

} // namespace kio
