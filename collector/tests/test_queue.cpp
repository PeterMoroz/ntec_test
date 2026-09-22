#include <gtest/gtest.h>

#include "bounded_queue.h"

#include <thread>
#include <vector>
#include <random>
#include <algorithm>

TEST(TestQueue, SingleProducersSingleConsumer)
{
    static const size_t QUEUE_MAX_LENGTH = 1024;
    BoundedQueue<int> queue(QUEUE_MAX_LENGTH);

    std::vector<int> d1, d2;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 1000000);

    const size_t N = 25'000;

    for (size_t i = 0; i < N; i++) {
        const int x = dist(rng);
        d1.push_back(x);
    }

    std::thread p = std::thread([&queue, &d1]() {
        for (size_t i = 0; i < d1.size(); i++) {
            int x = d1[i];
            queue.Put(std::move(x));
        }
    });

    std::thread c = std::thread([&queue, &d2]() {
        size_t consumed_cnt = 0;
        while (consumed_cnt < N) {
            int x = 0;
            queue.Get(x);
            consumed_cnt++;            
            d2.push_back(x);
        }
    });

    p.join();
    c.join();

    std::sort(d1.begin(), d1.end());
    std::sort(d2.begin(), d2.end());
    EXPECT_TRUE(d1 == d2);
}


TEST(TestQueue, MultipleProducersSingleConsumer)
{
    static const size_t QUEUE_MAX_LENGTH = 1024;
    BoundedQueue<int> queue(QUEUE_MAX_LENGTH);

    std::vector<int> d1, d2, d3, d4, d5;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 1000000);

    const size_t N = 25'000;

    for (size_t i = 0; i < N; i++) {
        const int x = dist(rng);
        d1.push_back(x);
    }

    for (size_t i = 0; i < N; i++) {
        const int x = dist(rng);
        d2.push_back(x);
    }

    for (size_t i = 0; i < N; i++) {
        const int x = dist(rng);
        d3.push_back(x);
    }

    for (size_t i = 0; i < N; i++) {
        const int x = dist(rng);
        d4.push_back(x);
    }


    std::thread p1 = std::thread([&queue, &d1]() {
        for (size_t i = 0; i < d1.size(); i++) {
            int x = d1[i];
            queue.Put(std::move(x));
        }
    });

    std::thread p2 = std::thread([&queue, &d2]() {
        for (size_t i = 0; i < d2.size(); i++) {
            int x = d2[i];
            queue.Put(std::move(x));
        }
    });

    std::thread p3 = std::thread([&queue, &d3]() {
        for (size_t i = 0; i < d3.size(); i++) {
            int x = d3[i];
            queue.Put(std::move(x));
        }
    });

    std::thread p4 = std::thread([&queue, &d4]() {
        for (size_t i = 0; i < d4.size(); i++) {
            int x = d4[i];
            queue.Put(std::move(x));
        }
    });

    std::thread c = std::thread([&queue, &d5]() {
        size_t consumed_cnt = 0;
        while (consumed_cnt < 4 * N) {
            int x = 0;
            queue.Get(x);
            consumed_cnt++;
            d5.push_back(x);
        }
    });

    p1.join();
    p2.join();
    p3.join();
    p4.join();
    c.join();

    d1.insert(d1.end(), d2.cbegin(), d2.cend());
    d1.insert(d1.end(), d3.cbegin(), d3.cend());
    d1.insert(d1.end(), d4.cbegin(), d4.cend());

    std::sort(d1.begin(), d1.end());
    std::sort(d5.begin(), d5.end());
    EXPECT_TRUE(d1 == d5);
}
