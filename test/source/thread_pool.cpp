#include <doctest/doctest.h>
#include <thread_pool/thread_pool.h>
// ensure version header is included properly with the project
#include <thread_pool/version.h>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <array>

auto multiply(int a, int b) { return a * b; }

TEST_CASE("Multiply using global function") {
    dp::thread_pool pool{};
    auto result = pool.enqueue(multiply, 3, 4);
    CHECK_EQ(result.get(), 12);
}

TEST_CASE("Multiply using lambda") {
    dp::thread_pool pool{};
    auto result = pool.enqueue([](int a, int b) { return a * b; }, 3, 4);
    CHECK_EQ(result.get(), 12);
}

TEST_CASE("Multiply with functor") {
    dp::thread_pool pool{};
    auto result = pool.enqueue(std::multiplies<int>{}, 3, 4);
    CHECK_EQ(result.get(), 12);
}

TEST_CASE("Pass reference to pool") {
    int x = 2;
    {
        dp::thread_pool pool{};
        pool.enqueue_detach([](int& a) { a *= 2; }, std::ref(x));
    }
    CHECK_EQ(x, 4);
}

TEST_CASE("Pass raw reference to pool") {
    int x = 2;
    {
        dp::thread_pool pool{};
        pool.enqueue_detach([](int& a) { a *= 2; }, x);
    }
    CHECK_EQ(x, 2);
}

TEST_CASE("Support enqueue with void return type") {
    dp::thread_pool pool{};
    auto value = 8;
    auto future = pool.enqueue([](int& x) { x *= 2; }, std::ref(value));
    future.wait();
    CHECK_EQ(value, 16);
}

TEST_CASE("Support enqueue_detach with void return type") {
    auto value = 8;
    {
        dp::thread_pool pool;
        pool.enqueue_detach([](int& x) { x *= 2; }, std::ref(value));
    }
    CHECK_EQ(value, 16);
}

TEST_CASE("Support enqueue_detach with non void return type") {
    auto value = 8;
    {
        dp::thread_pool pool;
        pool.enqueue_detach(
            [](int& x) {
                x *= 2;
                return x;
            },
            std::ref(value));
    }
    CHECK_EQ(value, 16);
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

TEST_CASE("Support params of different types") {
    dp::thread_pool pool{};
    struct test_struct {
        int value{};
        double d_value{};
    };

    test_struct test;

    auto task = [&test](int x, double y) -> test_struct {
        test.value = x;
        test.d_value = y;

        return test_struct{x, y};
    };

    auto future = pool.enqueue(task, 2, 3.2);
    const auto result = future.get();
    CHECK_EQ(result.value, test.value);
    CHECK_EQ(result.d_value, test.d_value);
}

TEST_CASE("Ensure work completes upon destruction") {
    std::atomic<int> counter;
    constexpr auto total_tasks = 30;
    {
        dp::thread_pool pool(4);
        for (auto i = 0; i < total_tasks; i++) {
            auto task = [i, &counter]() {
                std::this_thread::sleep_for(std::chrono::milliseconds((i + 1) * 10));
                ++counter;
            };
            pool.enqueue_detach(task);
        }
    }

    CHECK_EQ(counter.load(), total_tasks);
}

TEST_CASE("Ensure task load is spread evenly across threads") {
    auto delay_task = [](const std::chrono::seconds& seconds) {
        std::cout << std::this_thread::get_id() << " start : " << std::to_string(seconds.count())
                  << "\n";
        std::this_thread::sleep_for(seconds);
        std::cout << std::this_thread::get_id() << " end: " << std::to_string(seconds.count())
                  << "\n";
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
    std::cout << "total duration: " << duration.count() << "\n";
    // worst case is the same thread doing the long task back to back. Tasks are assigned
    // sequentially in the thread pool so this would be the default execution if there was no work
    // stealing.
    CHECK_LT(duration.count(), long_task_time * 2 + 1);
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
        dp::thread_pool pool{};

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

class might_throw_thread {
  public:
    explicit might_throw_thread() = default;
    template <class Function, class... Args>
    explicit might_throw_thread(Function&& func, Args&&... args,
                                const double& cut_off_probability = 0.5) {
        // generate random crossover points
        constexpr auto N = std::mt19937::state_size * sizeof(std::mt19937::result_type);
        std::random_device source;
        std::vector random_data(std::size_t(), (N - 1) / sizeof(source()) + 1);
        std::generate_n(random_data.begin(), random_data.size(), [&] { return source(); });
        std::seed_seq seeds(std::begin(random_data), std::end(random_data));

        static thread_local std::mt19937 device(seeds);
        std::uniform_real_distribution dist(0.0, 1.0);

        const auto& value = dist(device);
        if (value < cut_off_probability) throw std::system_error(std::error_code());

        impl_ = std::jthread(std::forward<Function>(func), std::forward<Args>(args)...);
    }
    ~might_throw_thread() { try_cancel_and_join(); }
    might_throw_thread(const might_throw_thread&) = delete;
    might_throw_thread(might_throw_thread&&) noexcept = default;
    might_throw_thread& operator=(const might_throw_thread&) = delete;
    might_throw_thread& operator=(might_throw_thread&& other) noexcept {
        if (this == std::addressof(other)) {
            return *this;
        }

        try_cancel_and_join();
        impl_ = std::move(other.impl_);
        return *this;
    }
    void swap(might_throw_thread& other) noexcept { std::swap(impl_, other.impl_); }

    void request_stop() { impl_.request_stop(); }
    void join() { impl_.join(); }

  private:
    void try_cancel_and_join() {
        if (impl_.joinable()) {
            impl_.request_stop();
            impl_.join();
        }
    }
    std::jthread impl_;
};

TEST_CASE("Create thread pool with fewer than requested threads") {
    const dp::thread_pool<dp::details::default_function_type, might_throw_thread> thread_pool{};
    CHECK_LT(thread_pool.size(), std::thread::hardware_concurrency());
}

TEST_CASE("Ensure work completes with fewer threads than expected.") {
    std::atomic counter = 0;
    int total_tasks{};

    SUBCASE("with tasks") { total_tasks = 30; }
    SUBCASE("with no tasks") { total_tasks = 0; }
    {
        dp::thread_pool<dp::details::default_function_type, might_throw_thread> pool(4);
        for (auto i = 0; i < total_tasks; i++) {
            auto task = [i, &counter]() {
                std::this_thread::sleep_for(std::chrono::milliseconds((i + 1) * 10));
                ++counter;
            };
            pool.enqueue_detach(task);
        }
    }

    CHECK_EQ(counter.load(), total_tasks);
}

TEST_CASE(
    "Ensure work completes when one thread is running, another is finished, and a new task is "
    "enqueued") {
    std::atomic<size_t> last_thread;

    {
        dp::thread_pool thread_pool{2};

        // tie up the first thread
        thread_pool.enqueue_detach([&last_thread]() {
            std::this_thread::sleep_for(std::chrono::seconds{5});
            last_thread = 1;
        });

        // run a quick job on the second thread
        thread_pool.enqueue_detach([&last_thread]() {
            std::this_thread::sleep_for(std::chrono::milliseconds{50});
            last_thread = 2;
        });

        // wait for the second thread to finish
        std::this_thread::sleep_for(std::chrono::seconds{1});

        // enqueue a quick job
        thread_pool.enqueue_detach([&last_thread]() {
            std::this_thread::sleep_for(std::chrono::milliseconds{50});
            last_thread = 3;
        });
    }

    CHECK_EQ(1, last_thread.load());
}

void recursive_sequential_sum(std::atomic_int32_t& counter, int count, dp::thread_pool<>& pool) {
    counter.fetch_add(count);
    if (count > 1) {
        pool.enqueue_detach(recursive_sequential_sum, std::ref(counter), count - 1, std::ref(pool));
    }
}

TEST_CASE("Recursive enqueue calls work correctly") {
    std::atomic_int32_t counter = 0;
    constexpr auto start = 1000;
    {
        dp::thread_pool pool(4);
        recursive_sequential_sum(counter, start, pool);
    }

    auto expected_sum = 0;
    for (int i = 0; i <= start; i++) {
        expected_sum += i;
    }
    CHECK_EQ(expected_sum, counter.load());
}

void recursive_parallel_sort(int* begin, int* end, int split_level, dp::thread_pool<>& pool) {
    if (split_level < 2 || end - begin < 2) {
        std::sort(begin, end);
    } else {
        const auto mid = begin + (end - begin) / 2;
        if (split_level == 2) {
            const auto future =
                pool.enqueue(recursive_parallel_sort, begin, mid, split_level / 2, std::ref(pool));
            std::sort(mid, end);
            future.wait();
        } else {
            const auto left =
                pool.enqueue(recursive_parallel_sort, begin, mid, split_level / 2, std::ref(pool));
            const auto right =
                pool.enqueue(recursive_parallel_sort, mid, end, split_level / 2, std::ref(pool));

            left.wait();
            right.wait();
        }
        std::inplace_merge(begin, mid, end);
    }
}

TEST_CASE("Recursive parallel sort") {
    std::vector<int> data(10000);
    // std::ranges::iota is a C++23 feature
    std::iota(data.begin(), data.end(), 0);
    std::ranges::shuffle(data, std::mt19937{std::random_device{}()});

    {
        dp::thread_pool pool(4);
        recursive_parallel_sort(data.data(), data.data() + data.size(), 4, pool);
    }

    CHECK(std::ranges::is_sorted(data));
}

TEST_CASE("Test premature exit") {
    // two threads in pool, thread1, thread2
    // first, push task_1
    // task_1 pushes task_2 and sleeps, so both threads are busy and no tasks are in queue
    // thread1 - task1, thread2 - task2
    // task_1 finishes, no tasks in queue, but task_2 is still running --> thread1 must not exit
    // task_2 pushes another task (end_task) and sleeps for 5s before finishing the task_2
    // So the first thread, thread1 should execute the end_task
    // but if the thread1 prematurely exits, than the end_task will be executed by the thread2

    std::thread::id id_task_1, id_end;
    {
        dp::thread_pool<> testPool(2);

        auto end = [&id_end]() { id_end = std::this_thread::get_id(); };

        auto task_2 = [&testPool, end]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            testPool.enqueue_detach(end);
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        };

        auto task_1 = [&testPool, &id_task_1, task_2]() {
            id_task_1 = std::this_thread::get_id();
            testPool.enqueue_detach(task_2);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        };

        testPool.enqueue_detach(task_1);
    }

    CHECK_EQ(id_task_1, id_end);

    // another scenario with 3 threads
    // 3 tasks started, so all get pushed to a single thread
    // threads 1 and 2 complete and there is no more work
    // but thread 3's task pushes a new task.
    // thread 3 sleeps right away so it's not available to handle the task, so thread 1 or 2 should
    // handle it
    std::thread::id spawn_task_id, task_1_id, task_2_id, task_3_id;
    {
        dp::thread_pool pool{3};

        using namespace std::chrono_literals;
        auto short_task = [] { std::this_thread::sleep_for(500ms); };
        auto long_task = [] { std::this_thread::sleep_for(2000ms); };
        auto task_1 = [&task_1_id, short_task] {
            task_1_id = std::this_thread::get_id();
            short_task();
        };
        auto task_2 = [&task_2_id, short_task] {
            task_2_id = std::this_thread::get_id();
            short_task();
        };

        auto spawned_task = [&spawn_task_id, short_task] {
            spawn_task_id = std::this_thread::get_id();
            short_task();
        };

        auto task_3 = [short_task, long_task, spawned_task, &task_3_id, &pool] {
            task_3_id = std::this_thread::get_id();
            short_task();
            pool.enqueue_detach(spawned_task);
            long_task();
        };

        pool.enqueue_detach(task_1);
        pool.enqueue_detach(task_2);
        pool.enqueue_detach(task_3);
    }

    // the task that spawns the new task should not run the new task
    CHECK_NE(spawn_task_id, task_3_id);
    CHECK_NE(task_1_id, task_2_id);
}

TEST_CASE("Ensure wait_for_tasks() properly blocks current execution.") {
    std::atomic counter = 0;
    int total_tasks{};
    constexpr auto thread_count = 4;

    SUBCASE("with tasks") { total_tasks = 30; }
    SUBCASE("with no tasks") { total_tasks = 0; }
    SUBCASE("with task count less than thread count") { total_tasks = thread_count / 2; }

    dp::thread_pool pool(thread_count);
    for (auto i = 0; i < total_tasks; i++) {
        auto task = [i, &counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds((i + 1) * 10));
            ++counter;
        };
        pool.enqueue_detach(task);
    }
    pool.wait_for_tasks();

    CHECK_EQ(counter.load(), total_tasks);
}

TEST_CASE("Ensure wait_for_tasks() properly waits for tasks to fully complete") {
    class counter_wrapper {
      public:
        std::atomic_int counter = 0;

        void increment_counter() { counter.fetch_add(1, std::memory_order_release); }
    };

    dp::thread_pool local_pool{};
    constexpr auto task_count = 10;
    std::array<int, task_count> counts{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    for (size_t i = 0; i < task_count; i++) {
        counter_wrapper cnt_wrp{};

        for (size_t var1 = 0; var1 < 17; var1++) {
            for (int var2 = 0; var2 < 12; var2++) {
                local_pool.enqueue_detach([&cnt_wrp]() { cnt_wrp.increment_counter(); });
            }
        }
        local_pool.wait_for_tasks();
        // std::cout << cnt_wrp.counter << std::endl;
        counts[i] = cnt_wrp.counter.load(std::memory_order_acquire);
    }

    auto all_correct_count =
        std::ranges::all_of(counts, [](int count) { return count == 17 * 12; });
    const auto sum = std::accumulate(counts.begin(), counts.end(), 0);
    CHECK_EQ(sum, 17 * 12 * task_count);
    CHECK(all_correct_count);
}

TEST_CASE("Ensure wait_for_tasks() can be called multiple times on the same pool") {
    class counter_wrapper {
      public:
        std::atomic_int counter = 0;

        void increment_counter() { counter.fetch_add(1, std::memory_order_release); }
    };

    dp::thread_pool local_pool{};
    constexpr auto task_count = 10;
    std::array<int, task_count> counts{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    for (size_t i = 0; i < task_count; i++) {
        counter_wrapper cnt_wrp{};

        for (size_t var1 = 0; var1 < 16; var1++) {
            for (int var2 = 0; var2 < 13; var2++) {
                local_pool.enqueue_detach([&cnt_wrp]() { cnt_wrp.increment_counter(); });
            }
        }
        local_pool.wait_for_tasks();
        // std::cout << cnt_wrp.counter << std::endl;
        counts[i] = cnt_wrp.counter.load(std::memory_order_acquire);
    }

    auto all_correct_count =
        std::ranges::all_of(counts, [](int count) { return count == 16 * 13; });
    auto sum = std::accumulate(counts.begin(), counts.end(), 0);
    CHECK_EQ(sum, 16 * 13 * task_count);
    CHECK(all_correct_count);

    for (size_t i = 0; i < task_count; i++) {
        counter_wrapper cnt_wrp{};

        for (size_t var1 = 0; var1 < 17; var1++) {
            for (int var2 = 0; var2 < 12; var2++) {
                local_pool.enqueue_detach([&cnt_wrp]() { cnt_wrp.increment_counter(); });
            }
        }
        local_pool.wait_for_tasks();
        // std::cout << cnt_wrp.counter << std::endl;
        counts[i] = cnt_wrp.counter.load(std::memory_order_acquire);
    }

    all_correct_count = std::ranges::all_of(counts, [](int count) { return count == 17 * 12; });
    sum = std::accumulate(counts.begin(), counts.end(), 0);
    CHECK_EQ(sum, 17 * 12 * task_count);
    CHECK(all_correct_count);
}

TEST_CASE("Initialization function is called") {
    std::atomic_int counter = 0;
    {
        dp::thread_pool pool(4, [&counter](std::size_t id) {
            std::cout << "Thread " << id << " initialized\n";
            counter.fetch_add(1);
        });
    }
    CHECK_EQ(counter.load(), 4);
}
