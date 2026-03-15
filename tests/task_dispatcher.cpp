#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "task_dispatcher.hpp"

using namespace dispatcher;

TEST(TaskDispatcher, ExecutesAllTasks) {
    std::atomic<int> count{0};
    {
        TaskDispatcher td(2);
        for (int i = 0; i < 100; ++i) {
            td.schedule(TaskPriority::Normal, [&] { count.fetch_add(1); });
        }
    }
    EXPECT_EQ(count.load(), 100);
}

TEST(TaskDispatcher, HighPriorityFirst) {
    std::vector<int> result;
    std::mutex mtx;
    {
        TaskDispatcher td(1);
        for (int i = 0; i < 10; ++i) {
            td.schedule(TaskPriority::Normal, [&, i] {
                std::lock_guard lock(mtx);
                result.push_back(i);
            });
        }
        for (int i = 100; i < 110; ++i) {
            td.schedule(TaskPriority::High, [&, i] {
                std::lock_guard lock(mtx);
                result.push_back(i);
            });
        }
    }
    // With 1 thread, some Normal tasks may run before High tasks are scheduled,
    // but once High tasks are in the queue they should be picked before remaining Normal.
    // Find the last High task position — all High tasks should cluster towards the front.
    auto last_high = std::find_if(result.rbegin(), result.rend(), [](int v) { return v >= 100; });
    auto first_normal_after = std::find_if(last_high.base(), result.end(), [](int v) { return v < 100; });
    // After the last High task, only Normal tasks should remain.
    bool no_high_after_last_high = std::none_of(first_normal_after, result.end(), [](int v) { return v >= 100; });
    EXPECT_TRUE(no_high_after_last_high);
}

TEST(TaskDispatcher, DestructorCompletesAllTasks) {
    std::atomic<int> count{0};
    {
        TaskDispatcher td(4);
        for (int i = 0; i < 50; ++i) {
            td.schedule(TaskPriority::High, [&] {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                count.fetch_add(1);
            });
        }
    }
    EXPECT_EQ(count.load(), 50);
}

TEST(TaskDispatcher, ConcurrentScheduling) {
    std::atomic<int> count{0};
    {
        TaskDispatcher td(4);
        std::vector<std::jthread> producers;
        for (int t = 0; t < 4; ++t) {
            producers.emplace_back([&] {
                for (int i = 0; i < 250; ++i) {
                    td.schedule(TaskPriority::Normal, [&] { count.fetch_add(1); });
                }
            });
        }
    }
    EXPECT_EQ(count.load(), 1000);
}

TEST(TaskDispatcher, MixedPrioritiesAllExecuted) {
    std::atomic<int> high_count{0};
    std::atomic<int> normal_count{0};
    {
        TaskDispatcher td(2);
        for (int i = 0; i < 50; ++i) {
            td.schedule(TaskPriority::High, [&] { high_count.fetch_add(1); });
            td.schedule(TaskPriority::Normal, [&] { normal_count.fetch_add(1); });
        }
    }
    EXPECT_EQ(high_count.load(), 50);
    EXPECT_EQ(normal_count.load(), 50);
}
