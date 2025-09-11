#include "kio/parallel_executor.hpp"
#include <iostream>
#include <future>
#include <algorithm>

namespace kio {

ParallelExecutor::ParallelExecutor() : numThreads(std::thread::hardware_concurrency()) {
    if (numThreads == 0) numThreads = 4; // Fallback
}

ParallelExecutor::~ParallelExecutor() {
    shutdown();
}

void ParallelExecutor::initialize() {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock, [this] { return !tasks.empty() || stop; });
                    
                    if (stop && tasks.empty()) return;
                    
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

void ParallelExecutor::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers.clear();
}

std::future<void> ParallelExecutor::execute(std::function<void()> task) {
    auto taskPtr = std::make_shared<std::packaged_task<void()>>(std::move(task));
    std::future<void> result = taskPtr->get_future();
    
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) {
            throw std::runtime_error("Cannot enqueue task on stopped executor");
        }
        tasks.emplace([taskPtr] { (*taskPtr)(); });
    }
    condition.notify_one();
    
    return result;
}

void ParallelExecutor::executeParallel(const std::vector<std::function<void()>>& tasks) {
    std::vector<std::future<void>> futures;
    
    for (const auto& task : tasks) {
        futures.push_back(execute(task));
    }
    
    // Wait for all tasks to complete
    for (auto& future : futures) {
        future.wait();
    }
}

size_t ParallelExecutor::getThreadCount() const {
    return numThreads;
}

} // namespace kio
