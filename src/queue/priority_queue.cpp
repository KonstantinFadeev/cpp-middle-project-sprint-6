#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(std::map<TaskPriority, QueueOptions> config) {
    for (auto &[priority, options] : config) {
        if (options.bounded) {
            if (!options.capacity.has_value()) {
                throw std::invalid_argument("BoundedQueue requires capacity");
            }
            queues_[priority] = std::make_unique<BoundedQueue>(options.capacity.value());
        } else {
            queues_[priority] = std::make_unique<UnboundedQueue>(options.capacity.value_or(0));
        }
    }
}

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    queues_.at(priority)->push(std::move(task));
    std::lock_guard lock(mutex_);
    cv_.notify_one();
}

std::optional<std::function<void()>> PriorityQueue::pop() {
    std::unique_lock lock(mutex_);
    while (true) {
        for (auto &[priority, queue] : queues_) {
            auto task = queue->try_pop();
            if (task) {
                return task;
            }
        }
        if (shutdown_.load(std::memory_order_relaxed)) {
            return std::nullopt;
        }
        cv_.wait(lock);
    }
}

void PriorityQueue::shutdown() {
    std::lock_guard lock(mutex_);
    shutdown_.store(true, std::memory_order_relaxed);
    cv_.notify_all();
}

PriorityQueue::~PriorityQueue() { shutdown(); }

}  // namespace dispatcher::queue
