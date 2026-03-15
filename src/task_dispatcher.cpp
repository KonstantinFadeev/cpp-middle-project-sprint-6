#include "task_dispatcher.hpp"

namespace dispatcher {

TaskDispatcher::TaskDispatcher(size_t thread_count, std::map<TaskPriority, queue::QueueOptions> config)
    : queue_(std::make_shared<queue::PriorityQueue>(std::move(config))), pool_(queue_, thread_count) {}

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) {
    queue_->push(priority, std::move(task));
}

TaskDispatcher::~TaskDispatcher() = default;

}  // namespace dispatcher
