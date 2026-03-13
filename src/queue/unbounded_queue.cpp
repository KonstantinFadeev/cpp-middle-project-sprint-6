#include "queue/unbounded_queue.hpp"

namespace dispatcher::queue {

UnboundedQueue::UnboundedQueue([[maybe_unused]] int capacity) {}

void UnboundedQueue::push(std::function<void()> task) {
    std::lock_guard lock(mutex_);
    queue_.push(std::move(task));
}

std::optional<std::function<void()>> UnboundedQueue::try_pop() {
    std::lock_guard lock(mutex_);
    if (queue_.empty()) {
        return std::nullopt;
    }
    auto task = std::move(queue_.front());
    queue_.pop();
    return task;
}

UnboundedQueue::~UnboundedQueue() = default;

}  // namespace dispatcher::queue
