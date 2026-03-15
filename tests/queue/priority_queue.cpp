#include <gtest/gtest.h>

#include <thread>

#include "queue/priority_queue.hpp"

using namespace dispatcher;
using namespace dispatcher::queue;

static std::map<TaskPriority, QueueOptions> default_config() {
    return {
        {TaskPriority::High, {.bounded = true, .capacity = 100}},
        {TaskPriority::Normal, {.bounded = false, .capacity = std::nullopt}},
    };
}

TEST(PriorityQueue, HighBeforeNormal) {
    PriorityQueue q(default_config());
    std::vector<int> result;

    q.push(TaskPriority::Normal, [&] { result.push_back(1); });
    q.push(TaskPriority::Normal, [&] { result.push_back(2); });
    q.push(TaskPriority::High, [&] { result.push_back(10); });
    q.push(TaskPriority::High, [&] { result.push_back(20); });

    q.shutdown();
    while (auto task = q.pop()) {
        (*task)();
    }
    EXPECT_EQ(result, (std::vector<int>{10, 20, 1, 2}));
}

TEST(PriorityQueue, PopBlocksUntilPush) {
    PriorityQueue q(default_config());
    std::atomic<bool> popped{false};

    std::jthread consumer([&] {
        auto task = q.pop();
        if (task) {
            (*task)();
        }
        popped.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(popped.load());

    q.push(TaskPriority::Normal, [] {});
    consumer.join();
    EXPECT_TRUE(popped.load());
}

TEST(PriorityQueue, ShutdownUnblocksPopWithNullopt) {
    PriorityQueue q(default_config());
    std::optional<std::function<void()>> result;

    std::jthread consumer([&] { result = q.pop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    q.shutdown();
    consumer.join();
    EXPECT_FALSE(result.has_value());
}

TEST(PriorityQueue, ShutdownDrainsRemaining) {
    PriorityQueue q(default_config());
    std::atomic<int> count{0};

    q.push(TaskPriority::Normal, [&] { count.fetch_add(1); });
    q.push(TaskPriority::High, [&] { count.fetch_add(1); });
    q.shutdown();

    while (auto task = q.pop()) {
        (*task)();
    }
    EXPECT_EQ(count.load(), 2);
}

TEST(PriorityQueue, FifoWithinSamePriority) {
    PriorityQueue q(default_config());
    std::vector<int> result;

    for (int i = 0; i < 5; ++i) {
        q.push(TaskPriority::High, [&result, i] { result.push_back(i); });
    }
    q.shutdown();
    while (auto task = q.pop()) {
        (*task)();
    }
    EXPECT_EQ(result, (std::vector<int>{0, 1, 2, 3, 4}));
}
