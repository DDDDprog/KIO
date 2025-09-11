/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <vector>
#include <memory>
#include <future>
#include <thread>
#include <functional>

namespace kio {

class Value; // Forward declaration
class Stmt; // Forward declaration

class ParallelExecutor {
public:
    ParallelExecutor(size_t thread_count = std::thread::hardware_concurrency());
    ~ParallelExecutor();
    
    // Execute statements in parallel
    void executeParallel(const std::vector<std::unique_ptr<Stmt>>& statements);
    
    // Execute functions in parallel
    std::vector<Value> executeParallelFunctions(
        const std::vector<std::function<Value()>>& functions
    );
    
    // Wait for all tasks to complete
    void waitForAll();
    
    // Get number of worker threads
    size_t getThreadCount() const { return thread_count_; }
    
    // Check if executor is busy
    bool isBusy() const;

private:
    size_t thread_count_;
    std::vector<std::thread> workers_;
    std::vector<std::future<void>> futures_;
    bool shutdown_;
    
    void initializeWorkers();
    void shutdownWorkers();
};

} // namespace kio
