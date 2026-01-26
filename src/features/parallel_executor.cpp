/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/parallel_executor.hpp"
#include "kio/ast.hpp"
#include <iostream>
#include <algorithm>

namespace kio {

ParallelExecutor::ParallelExecutor(size_t thread_count) 
    : thread_count_(thread_count), shutdown_(false) {
    if (thread_count_ == 0) thread_count_ = 4; // Fallback
    initializeWorkers();
}

ParallelExecutor::~ParallelExecutor() {
    shutdownWorkers();
}

void ParallelExecutor::initializeWorkers() {
    // Initialize worker threads if needed
    std::cout << "Parallel executor initialized with " << thread_count_ << " threads" << std::endl;
}

void ParallelExecutor::shutdownWorkers() {
    shutdown_ = true;
    waitForAll();
}

void ParallelExecutor::executeParallel(const std::vector<std::unique_ptr<Stmt>>& statements) {
    // Basic parallel execution stub
    std::cout << "Executing " << statements.size() << " statements in parallel" << std::endl;
}

std::vector<Value> ParallelExecutor::executeParallelFunctions(
    const std::vector<std::function<Value()>>& functions) {
    std::vector<Value> results;
    std::vector<std::future<Value>> futures;
    
    for (const auto& func : functions) {
        futures.push_back(std::async(std::launch::async, func));
    }
    
    for (auto& future : futures) {
        results.push_back(future.get());
    }
    
    return results;
}

void ParallelExecutor::waitForAll() {
    for (auto& future : futures_) {
        if (future.valid()) {
            future.wait();
        }
    }
    futures_.clear();
}

bool ParallelExecutor::isBusy() const {
    return !futures_.empty();
}

} // namespace kio
