#pragma once
#include "queue/queue.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
    std::queue<std::function<void()>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_;
    int capacity_;

public:
    explicit BoundedQueue(int capacity);

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~BoundedQueue() override;
};

}  // namespace dispatcher::queue