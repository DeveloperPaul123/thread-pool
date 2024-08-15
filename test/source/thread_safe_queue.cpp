#include <doctest/doctest.h>
#include <thread_pool/thread_safe_queue.h>

#include <barrier>
#include <future>
#include <thread>

TEST_CASE("Ensure insert and pop works with thread contention") {
    // create a synchronization barrier to ensure our threads have started before executing code to
    // access the queue
    std::barrier barrier(3);
    std::promise<int> p1, p2, p3;
    {
        // initialize the queue
        dp::thread_safe_queue<int> queue;
        std::jthread t1([&queue, &barrier, &p1] {
            barrier.arrive_and_wait();
            queue.push_front(1);
            barrier.arrive_and_wait();
            p1.set_value(queue.pop_back().value_or(-1));
        });
        std::jthread t2([&queue, &barrier, &p2] {
            barrier.arrive_and_wait();
            queue.push_front(2);
            barrier.arrive_and_wait();
            p2.set_value(queue.pop_back().value_or(-1));
        });
        std::jthread t3([&queue, &barrier, &p3] {
            barrier.arrive_and_wait();
            queue.push_front(3);
            barrier.arrive_and_wait();
            p3.set_value(queue.pop_back().value_or(-1));
        });
    }

    // get the popped values
    auto fut1 = p1.get_future();
    auto fut2 = p2.get_future();
    auto fut3 = p3.get_future();
    auto res1 = fut1.get();
    auto res2 = fut2.get();
    auto res3 = fut3.get();

    // ensure that we got a value
    CHECK_NE(res1, -1);
    CHECK_NE(res2, -1);
    CHECK_NE(res3, -1);

    // we don't know what order that the values were pushed into the queue
    CHECK_NE(res1, res2);
    CHECK_NE(res2, res3);
    CHECK_NE(res3, res1);
}

TEST_CASE("Ensure clear() works and returns correct count") {
    // create a synchronization barrier to ensure our threads have started before executing code to
    // clear the queue

    // here, we check that:
    // - the queue is cleared
    // - that clear() return the correct number

    std::barrier barrier(3);
    std::atomic<size_t> removed_count{0};

    dp::thread_safe_queue<int> queue;
    {
        std::jthread t1([&queue, &barrier, &removed_count] {
            queue.push_front(1);
            barrier.arrive_and_wait();
            removed_count = queue.clear();
            barrier.arrive_and_wait();
        });
        std::jthread t2([&queue, &barrier] {
            queue.push_front(2);
            barrier.arrive_and_wait();
            barrier.arrive_and_wait();
        });
        std::jthread t3([&queue, &barrier] {
            queue.push_front(3);
            barrier.arrive_and_wait();
            barrier.arrive_and_wait();
        });
    }

    CHECK(queue.empty());
    CHECK_EQ(removed_count, 3);
}
