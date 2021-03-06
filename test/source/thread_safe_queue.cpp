#include <doctest/doctest.h>
#include <thread_pool/thread_safe_queue.h>

#include <future>

TEST_CASE("Ensure insert and pop works with thread contention") {
    dp::thread_safe_queue<int> queue;

    auto fut1 = std::async([&queue]() { queue.push(1); });
    auto fut2 = std::async([&queue]() { queue.push(2); });
    auto fut3 = std::async([&queue]() { queue.push(3); });

    auto fut4 = std::async([&queue]() { return queue.pop(); });
    auto fut5 = std::async([&queue]() { return queue.pop(); });
    auto fut6 = std::async([&queue]() { return queue.pop(); });

    fut1.get(), fut2.get(), fut3.get();

    auto res4 = fut4.get();
    auto res5 = fut5.get();
    auto res6 = fut6.get();

    CHECK(res4.has_value());
    CHECK(res5.has_value());
    CHECK(res6.has_value());

    // we don't know what order that the values were pushed into the queue
    CHECK_NE(res4.value(), res5.value());
    CHECK_NE(res5.value(), res6.value());
    CHECK_NE(res6.value(), res4.value());
}
