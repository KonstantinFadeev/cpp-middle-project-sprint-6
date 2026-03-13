#include <gtest/gtest.h>

#include <thread>

#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;

TEST(BoundedQueue, PushPopSingle) {
    BoundedQueue q(5);
    int value = 0;
    q.push([&] { value = 42; });
    auto task = q.try_pop();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(value, 42);
}

TEST(BoundedQueue, TryPopEmpty) {
    BoundedQueue q(5);
    auto task = q.try_pop();
    EXPECT_FALSE(task.has_value());
}

TEST(BoundedQueue, FifoOrder) {
    BoundedQueue q(10);
    std::vector<int> result;
    for (int i = 0; i < 5; ++i) {
        q.push([&result, i] { result.push_back(i); });
    }
    while (auto task = q.try_pop()) {
        (*task)();
    }
    EXPECT_EQ(result, (std::vector<int>{0, 1, 2, 3, 4}));
}

TEST(BoundedQueue, InvalidCapacity) {
    EXPECT_THROW(BoundedQueue(0), std::invalid_argument);
    EXPECT_THROW(BoundedQueue(-1), std::invalid_argument);
}

TEST(BoundedQueue, BlocksWhenFull) {
    BoundedQueue q(2);
    q.push([] {});
    q.push([] {});

    std::atomic<bool> pushed{false};
    std::jthread producer([&] {
        q.push([] {});
        pushed.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(pushed.load());

    q.try_pop();
    producer.join();
    EXPECT_TRUE(pushed.load());
}
