#include <doctest/doctest.h>
#include <thread_pool/thread_safe_queue.h>

#include <thread>

TEST_CASE("Check size while waiting for front") {
    // start with an empty queue
    dp::thread_safe_queue<int> queue{};
    int item = 0;
    std::jthread wait_for_item_thread([&item, &queue]() {
        // this will block until an item becomes available
        item = queue.front();
    });

    if (queue.empty()) {
        queue.push(3);
    }

    CHECK(queue.size() == 1);
    // wait for thread to finish it's work
    wait_for_item_thread.request_stop();
    wait_for_item_thread.join();
    CHECK(item == 3);
}
