#include <doctest/doctest.h>
#include <nanobench.h>
#include <thread_pool/thread_pool.h>

#include <chrono>
#include <riften/thiefpool.hpp>
#include <thread>

inline void thread_task() {
    int a = 0;
    int b = 1;

#pragma unroll
    for (int i = 0; i < 50; ++i) {
#pragma unroll
        for (int j = 0; j < 25; ++j) {
            a = a + b;
            b = b + a;
        }
    }
    int result = b;
    // ankerl::nanobench::doNotOptimizeAway(result);
}

// tests how well the thread pool scales for a given task
TEST_CASE("dp::thread_pool scaling") {
    using namespace std::chrono_literals;
    ankerl::nanobench::Bench bench;
    const auto bench_title = std::string("equilibrium 64,000");

    // clang-format off
    bench.title(bench_title)
        .warmup(10)
        .minEpochIterations(10)
        .relative(true)
        .timeUnit(1ms, "ms");
    // clang-format on

    for (unsigned int n_threads = 1; n_threads <= std::thread::hardware_concurrency();
         n_threads++) {
        const std::string run_title = "dp::thread_pool n_threads: " + std::to_string(n_threads);
        dp::thread_pool pool{n_threads};
        std::vector<std::future<void>> results(64'000);
        bench.run(run_title, [&] {
            for (auto i = 0; i < 64'000; i++) {
                results[i] = pool.enqueue(thread_task);
            }
            for (auto& result : results) result.get();
        });
        results.clear();
    }
}

TEST_CASE("riften::ThiefPool scaling") {
    using namespace std::chrono_literals;
    ankerl::nanobench::Bench bench;
    const auto bench_title = std::string("equilibrium 64,000");

    // clang-format off
    bench.title(bench_title)
        .warmup(10)
        .minEpochIterations(100)
        .relative(true)
        .timeUnit(1ms, "ms");
    // clang-format on

    for (unsigned int n_threads = 1; n_threads <= std::thread::hardware_concurrency();
         n_threads++) {
        const std::string run_title = "riften::ThiefPool n_threads: " + std::to_string(n_threads);

        bench.run(run_title, [=] {
            riften::Thiefpool pool(n_threads);

            for (auto i = 0; i < 64'000; i++) {
                pool.enqueue_detach(thread_task);
            }
        });
    }
}
