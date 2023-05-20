#include <doctest/doctest.h>
#include <nanobench.h>
#include <thread_pool/thread_pool.h>

#include <BS_thread_pool.hpp>
#include <concepts>
#include <fstream>
#include <future>
#include <riften/thiefpool.hpp>
#include <task_thread_pool.hpp>

#include "utilities.h"

template <std::integral DataType, typename FutureProvider>
    requires std::invocable<FutureProvider, std::vector<DataType>, std::vector<DataType>>
void run_benchmark(ankerl::nanobench::Bench* bench, const std::size_t array_size,
                   const std::size_t multiplications_to_perform, const char* name,
                   FutureProvider&& provider) {
    // generate test data
    // const auto computations = generate_benchmark_data<int>(array_size,
    // multiplications_to_perform);
    const std::vector<DataType> a(array_size, 2);
    const std::vector<DataType> b(array_size, 3);

    // set up vector for results
    std::vector<std::future<void>> results;

    if constexpr (!std::is_void_v<std::invoke_result_t<FutureProvider, std::vector<DataType>,
                                                       std::vector<DataType>>>) {
        results.reserve(multiplications_to_perform);
    }

    bench->run(name, [&]() {
        for (std::size_t i = 0; i < multiplications_to_perform; ++i) {
            // let std async decide on how to launch the task, either deferred or async
            if constexpr (std::is_same_v<std::future<void>,
                                         std::invoke_result_t<FutureProvider, std::vector<DataType>,
                                                              std::vector<DataType>>>) {
                results.emplace_back(provider(std::ref(a), std::ref(b)));

            } else {
                provider(std::ref(a), std::ref(b));
            }
        }

        if (!results.empty()) {
            // wait for futures
            for (auto& fut : results) {
                fut.wait();
            }
        }
    });
}

TEST_CASE("matrix_multiplication") {
    using namespace std::chrono_literals;

    // task that runs on a new thread (potentially)
    auto thread_task = [](const std::vector<int>& a, const std::vector<int>& b) {
        std::vector<int> result(a.size());
        multiply_array(a, b, result);
        ankerl::nanobench::doNotOptimizeAway(result);
    };

    std::vector<std::pair<std::size_t, std::size_t>> args = {
        {8, 100'000}, {64, 75'000}, {256, 50'000}, {512, 35'000}, {1024, 25'000}};

    std::ofstream output_file(RESULTS_MARKDOWN_FILE, std::ios::out);

    for (const auto& [array_size, iterations] : args) {
        ankerl::nanobench::Bench bench;
        auto bench_title = std::string("matrix multiplication ") + std::to_string(array_size) +
                           "x" + std::to_string(array_size);
        bench.title(bench_title)
            .warmup(10)
            .relative(true)
            .minEpochIterations(15)
            .output(&output_file);
        bench.timeUnit(1ms, "ms");

        {
            dp::thread_pool<std::function<void()>> pool{};
            run_benchmark<int>(&bench, array_size, iterations, "dp::thread_pool - std::function",
                               [&](const std::vector<int>& a, const std::vector<int>& b) -> void {
                                   pool.enqueue_detach(thread_task, a, b);
                               });
        }

#ifdef __cpp_lib_move_only_function
        {
            dp::thread_pool pool{};
            run_benchmark<int>(&bench, array_size, iterations,
                               "dp::thread_pool - std::move_only_function",
                               [&](const std::vector<int>& a, const std::vector<int>& b) -> void {
                                   pool.enqueue_detach(thread_task, a, b);
                               });
        }
#endif
        {
            dp::thread_pool<fu2::unique_function<void()>> pool{};
            run_benchmark<int>(&bench, array_size, iterations,
                               "dp::thread_pool - fu2::unique_function",
                               [&](const std::vector<int>& a, const std::vector<int>& b) -> void {
                                   pool.enqueue_detach(thread_task, a, b);
                               });
        }

        {
            BS::thread_pool bs_thread_pool{std::thread::hardware_concurrency()};
            run_benchmark<int>(&bench, array_size, iterations, "BS::thread_pool",
                               [&](const std::vector<int>& a, const std::vector<int>& b) -> void {
                                   bs_thread_pool.push_task(thread_task, a, b);
                               });
        }

        {
            task_thread_pool::task_thread_pool ttp{};
            run_benchmark<int>(&bench, array_size, iterations, "task_thread_pool",
                               [&](const std::vector<int>& a, const std::vector<int>& b) -> void {
                                   ttp.submit_detach(thread_task, a, b);
                               });
        }

        {
            riften::Thiefpool riften_thiefpool{};
            run_benchmark<int>(&bench, array_size, iterations, "riften::Thiefpool",
                               [&](const std::vector<int>& a, const std::vector<int>& b) -> void {
                                   riften_thiefpool.enqueue_detach(thread_task, a, b);
                               });
        }
    }
}
