#include <gtest/gtest.h>

#include <thread>

#include "queue/unbounded_queue.hpp"

using namespace dispatcher::queue;

TEST(UnboundedQueue, PushPopSingle) {
    UnboundedQueue q(0);
    int value = 0;
    q.push([&] { value = 42; });
    auto task = q.try_pop();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(value, 42);
}

TEST(UnboundedQueue, TryPopEmpty) {
    UnboundedQueue q(0);
    auto task = q.try_pop();
    EXPECT_FALSE(task.has_value());
}

TEST(UnboundedQueue, FifoOrder) {
    UnboundedQueue q(0);
    std::vector<int> result;
    for (int i = 0; i < 5; ++i) {
        q.push([&result, i] { result.push_back(i); });
    }
    while (auto task = q.try_pop()) {
        (*task)();
    }
    EXPECT_EQ(result, (std::vector<int>{0, 1, 2, 3, 4}));
}

TEST(UnboundedQueue, NeverBlocks) {
    UnboundedQueue q(0);
    for (int i = 0; i < 10000; ++i) {
        q.push([] {});
    }
    int count = 0;
    while (q.try_pop()) {
        ++count;
    }
    EXPECT_EQ(count, 10000);
}

TEST(UnboundedQueue, ConcurrentPushPop) {
    UnboundedQueue q(0);
    std::atomic<int> sum{0};
    constexpr int n = 1000;

    std::jthread producer([&] {
        for (int i = 0; i < n; ++i) {
            q.push([&sum] { sum.fetch_add(1); });
        }
    });
    producer.join();

    while (auto task = q.try_pop()) {
        (*task)();
    }
    EXPECT_EQ(sum.load(), n);
}
