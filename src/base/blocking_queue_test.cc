#include "base/blocking_queue.h"

#include <thread>
#include <gtest/gtest.h>

#include "base/wait_group.h"

TEST(BlockingQueue, basic)
{
    base::BlockingQueue<int> q(10);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(10U, q.capacity());
    EXPECT_EQ(0U, q.size());
    EXPECT_EQ(10U, q.available());

    q.push(3);
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(10U, q.capacity());
    EXPECT_EQ(1U, q.size());
    EXPECT_EQ(9U, q.available());

    q.push(5);
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(10U, q.capacity());
    EXPECT_EQ(2U, q.size());
    EXPECT_EQ(8U, q.available());

    EXPECT_EQ(3, q.take());
    EXPECT_EQ(5, q.take());

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(10U, q.capacity());
    EXPECT_EQ(0U, q.size());
    EXPECT_EQ(10U, q.available());
}

TEST(BlockingQueue, producer_consumer)
{
    base::BlockingQueue<int> q(10);
    WaitGroup wg;
    wg.add(2);

    std::thread producer([&q, &wg]() {
        for (int i = 0; i < 100; ++i) {
            q.push(i);
        }
        wg.done();
    });

    std::thread consumer([&q, &wg]() {
        for (int i = 0; i < 100; ++i) {
            EXPECT_EQ(i, q.take());
        }
        wg.done();
    });

    wg.waitUntilDone();
    producer.join();
    consumer.join();
}
