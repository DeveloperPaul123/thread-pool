#include <doctest/doctest.h>
#include <thread_pool/thread_pool.h>

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
