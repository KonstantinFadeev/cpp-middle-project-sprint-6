#pragma once

#include "queue/priority_queue.hpp"

#include <memory>
#include <thread>
#include <vector>

namespace dispatcher::thread_pool {

class ThreadPool {
    std::shared_ptr<queue::PriorityQueue> queue_;
    std::vector<std::jthread> workers_;

    void run();

public:
    ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count);
    ~ThreadPool();
};

}  // namespace dispatcher::thread_pool
