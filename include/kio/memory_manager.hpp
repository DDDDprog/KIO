/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <mutex>

namespace kio {

class MemoryManager {
public:
    MemoryManager(size_t initial_pool_size = 1024 * 1024); // 1MB default
    ~MemoryManager();
    
    // Fast allocation from memory pools
    void* allocate(size_t size, size_t alignment = 8);
    void deallocate(void* ptr);
    
    // Garbage collection
    void collectGarbage();
    void setGCThreshold(size_t threshold);
    
    // Memory statistics
    size_t getTotalAllocated() const;
    size_t getPoolSize() const;
    size_t getFragmentation() const;
    
    // Memory limits
    void setMemoryLimit(size_t limit);
    bool isMemoryLimitExceeded() const;

private:
    struct MemoryPool;
    std::vector<std::unique_ptr<MemoryPool>> pools_;
    
    std::atomic<size_t> total_allocated_{0};
    std::atomic<size_t> memory_limit_{SIZE_MAX};
    std::atomic<size_t> gc_threshold_{1024 * 1024}; // 1MB
    
    mutable std::mutex allocation_mutex_;
    
    void expandPool();
    void compactPools();
};

} // namespace kio
