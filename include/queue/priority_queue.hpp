#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace dispatcher::queue {

class PriorityQueue {
    std::map<TaskPriority, std::unique_ptr<IQueue>> queues_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> shutdown_{false};

public:
    explicit PriorityQueue(std::map<TaskPriority, QueueOptions> config);

    void push(TaskPriority priority, std::function<void()> task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<std::function<void()>> pop();

    void shutdown();

    ~PriorityQueue();
};

}  // namespace dispatcher::queue