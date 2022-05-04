#include <doctest/doctest.h>
#include <thread_pool/thread_pool.h>

#include <iostream>
#include <string>

auto multiply(int a, int b) { return a * b; }

TEST_CASE("Multiply using global function") {
    dp::thread_pool pool;
    auto result = pool.enqueue(multiply, 3, 4);
    CHECK_EQ(result.get(), 12);
}

TEST_CASE("Multiply using lambda") {
    dp::thread_pool pool;
    auto result = pool.enqueue([](int a, int b) { return a * b; }, 3, 4);
    CHECK_EQ(result.get(), 12);
}

TEST_CASE("Multiply with functor") {
    dp::thread_pool pool;
    auto result = pool.enqueue(std::multiplies<int>{}, 3, 4);
    CHECK_EQ(result.get(), 12);
}

TEST_CASE("Pass references to pool") {
    int x = 2;
    {
        dp::thread_pool pool;
        pool.enqueue_detach([](int& a) { a = 4; }, std::ref(x));
    }
    CHECK_EQ(x, 4);
}

TEST_CASE("Pass raw reference to pool") {
    int x = 2;
    {
        dp::thread_pool pool;
        pool.enqueue_detach([](int& a) { a *= 2; }, x);
    }
    CHECK_EQ(x, 2);
}

TEST_CASE("Ensure input params are properly passed") {
    dp::thread_pool pool(4);
    constexpr auto total_tasks = 30;
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
    constexpr auto total_tasks = 20;
    {
        dp::thread_pool pool(4);
        for (auto i = 0; i < total_tasks; i++) {
            auto task = [i, &counter]() {
                std::this_thread::sleep_for(std::chrono::milliseconds((i + 1) * 100));
                ++counter;
            };
            pool.enqueue_detach(task);
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

TEST_CASE("Ensure task exception doesn't kill worker thread") {
    auto throw_task = [](int) -> int { throw std::logic_error("Error occurred."); };
    auto regular_task = [](int input) -> int { return input * 2; };

    std::atomic_uint_fast64_t count(0);

    auto throw_no_return = []() { throw std::logic_error("Error occurred."); };
    auto no_throw_no_return = [&count]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count += 1;
    };

    {
        dp::thread_pool pool;

        auto throw_future = pool.enqueue(throw_task, 1);
        auto no_throw_future = pool.enqueue(regular_task, 2);

        CHECK_THROWS(throw_future.get());
        CHECK_EQ(no_throw_future.get(), 4);

        // do similar check for tasks without return
        pool.enqueue_detach(throw_no_return);
        pool.enqueue_detach(no_throw_no_return);
    }

    CHECK_EQ(count.load(), 1);
}
