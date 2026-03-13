#include "queue/bounded_queue.hpp"

#include <stdexcept>

namespace dispatcher::queue {

BoundedQueue::BoundedQueue(int capacity) : capacity_(capacity) {
    if (capacity_ <= 0) {
        throw std::invalid_argument("BoundedQueue capacity must be positive");
    }
}

void BoundedQueue::push(std::function<void()> task) {
    std::unique_lock lock(mutex_);
    not_full_.wait(lock, [this] { return static_cast<int>(queue_.size()) < capacity_; });
    queue_.push(std::move(task));
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    std::lock_guard lock(mutex_);
    if (queue_.empty()) {
        return std::nullopt;
    }
    auto task = std::move(queue_.front());
    queue_.pop();
    not_full_.notify_one();
    return task;
}

BoundedQueue::~BoundedQueue() = default;

}  // namespace dispatcher::queue
