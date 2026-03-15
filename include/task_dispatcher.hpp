#pragma once

#include <map>
#include <memory>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
    std::shared_ptr<queue::PriorityQueue> queue_;
    thread_pool::ThreadPool pool_;

public:
    TaskDispatcher(
        size_t thread_count,
        std::map<TaskPriority, queue::QueueOptions> config = {
            {TaskPriority::High, {.bounded = true, .capacity = 1000}},
            {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}}});

    void schedule(TaskPriority priority, std::function<void()> task);
    ~TaskDispatcher();
};

}  // namespace dispatcher