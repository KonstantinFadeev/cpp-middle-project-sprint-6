#include "thread_pool/thread_pool.hpp"

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count)
    : queue_(std::move(queue)) {
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { run(); });
    }
}

void ThreadPool::run() {
    while (auto task = queue_->pop()) {
        (*task)();
    }
}

ThreadPool::~ThreadPool() {
    queue_->shutdown();
    for (auto &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

}  // namespace dispatcher::thread_pool
