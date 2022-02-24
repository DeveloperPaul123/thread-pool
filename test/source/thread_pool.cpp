#include <doctest/doctest.h>
#include <thread_pool/thread_pool.h>

#include <iostream>
#include <string>

TEST_CASE("Basic task return types") {
    dp::thread_pool pool(2);
    auto future_value = pool.enqueue([](const int& value) { return value; }, 30);
    auto future_negative = pool.enqueue([](int x) -> int { return x - 20; }, 3);

    auto value1 = future_value.get();
    auto value2 = future_negative.get();
    CHECK(value1 == 30);
    CHECK(value2 == 3 - 20);
}

TEST_CASE("Ensure input params are properly passed") {
    dp::thread_pool pool(4);
    const auto total_tasks = 30;
    std::vector<std::future<int>> futures;

    for (auto i = 0; i < total_tasks; i++) {
        auto task = [index = i]() { return index; };

        futures.push_back(pool.enqueue(task));
    }

    for (auto j = 0; j < total_tasks; j++) {
        CHECK(j == futures[j].get());
    }
}

TEST_CASE("Ensure work completes upon destruction") {
    std::atomic<int> counter;
    std::vector<std::future<int>> futures;
    const auto total_tasks = 20;
    {
        dp::thread_pool pool(4);
        for (auto i = 0; i < total_tasks; i++) {
            auto task = [index = i, &counter]() {
                counter++;
                return index;
            };
            futures.push_back(pool.enqueue(task));
        }
    }

    CHECK_EQ(counter.load(), total_tasks);
}

TEST_CASE("Ensure task load is spread evenly across threads") {
    auto delay_task = [](const std::chrono::seconds& seconds) {
        std::this_thread::sleep_for(seconds);
    };
    constexpr auto long_task_time = 6;
    const auto start_time = std::chrono::steady_clock::now();
    {
        dp::thread_pool pool(4);
        for (auto i = 1; i <= 8; ++i) {
            auto delay_amount = std::chrono::seconds(i % 4);
            if (i % 4 == 0) {
                delay_amount = std::chrono::seconds(long_task_time);
            }
            std::cout << std::to_string(delay_amount.count()) << "\n";
            pool.enqueue_detach(delay_task, delay_amount);
        }
        // wait for tasks to complete
    }
    const auto end_time = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    /**
     * Potential execution graph
     * '-' and '*' represent task time.
     * '-' is the first round of tasks and '*' is the second round of tasks
     *
     * - * **
     * -- ***
     * --- ******
     * ------
     */
    CHECK_LE(duration.count(), 9);
    std::cout << "total duration: " << duration.count() << "\n";
}
