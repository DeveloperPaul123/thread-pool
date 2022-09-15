#include <benchmark/benchmark.h>
#include <thread_pool/thread_pool.h>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>

#include "utilities.h"

static void BM_array_multiplication_thread_pool(benchmark::State& state) {
    const auto array_size = state.range(0);
    const std::size_t multiplications_to_perform = state.range(1);

    // generate the data
    const auto computations = generate_benchmark_data<int>(array_size, multiplications_to_perform);

    // task that is run on a new thread
    auto thread_task = [](multiplication_pair<int> pair) {
        std::vector<int> result(pair.first.size());
        multiply_array(pair.first, pair.second, result);
    };

    // create our thread pool using the default size
    dp::thread_pool pool{};

    for (auto _ : state) {
        {
            for (const auto& mult : computations) {
                pool.enqueue_detach(thread_task, mult);
            }
        }
    }
}

BENCHMARK(BM_array_multiplication_thread_pool)
#if defined(NDEBUG)
    ->Args({8, 25'000})
    ->Args({64, 5'000})
    ->Args({256, 250})
    ->Args({512, 75})
    ->Args({1024, 10})
#else
    ->Args({8, 50})
#endif
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly(true)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("dp::thread_pool array mult");

static void BM_array_multiplication_batch_thread_pool(benchmark::State& state) {
    const auto array_size = state.range(0);
    const std::size_t multiplications_to_perform = state.range(1);

    // generate the data
    const auto computations = generate_benchmark_data<int>(array_size, multiplications_to_perform);

    // create our tasks
    std::vector<std::function<void()>> tasks{};
    tasks.reserve(computations.size());

    // task that is run on a new thread
    auto thread_task = [](multiplication_pair<int> pair) {
        std::vector<int> result(pair.first.size());
        multiply_array(pair.first, pair.second, result);
    };

    for (const auto& computation : computations) {
        auto task = [comp = computation, execution_task = thread_task]() { execution_task(comp); };
        tasks.emplace_back(task);
    }

    // create our thread pool using the default size
    dp::thread_pool pool{};

    for (auto _ : state) {
        pool.enqueue(tasks.begin(), tasks.end());
    }
}

BENCHMARK(BM_array_multiplication_batch_thread_pool)
#if defined(NDEBUG)
    ->Args({8, 25'000})
    ->Args({64, 5'000})
    ->Args({256, 250})
    ->Args({512, 75})
    ->Args({1024, 10})
#else
    ->Args({8, 50})
#endif
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly(true)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("dp::thread_pool batched array mult");

inline cppcoro::task<void> mult_task(multiplication_pair<int> pair, dp::thread_pool<>& pool) {
    std::vector<int> result(pair.first.size());
    co_await co_multiply_array(pair.first, pair.second, result, pool);
}

static void BM_array_multiplication_thread_pool_coroutine(benchmark::State& state) {
    const auto array_size = state.range(0);
    const std::size_t multiplications_to_perform = state.range(1);
    const auto computations = generate_benchmark_data<int>(array_size, multiplications_to_perform);
    dp::thread_pool pool{};
    std::vector<cppcoro::task<>> tasks;
    for (auto _ : state) {
        for (auto mult_pair : computations) {
            tasks.emplace_back(mult_task(mult_pair, pool));
        }
    }

    cppcoro::sync_wait(cppcoro::when_all(std::move(tasks)));
}

BENCHMARK(BM_array_multiplication_thread_pool_coroutine)
#if defined(NDEBUG)
    ->Args({8, 25'000})
    ->Args({64, 5'000})
    ->Args({256, 250})
    ->Args({512, 75})
    ->Args({1024, 10})
#else
    ->Args({8, 50})
#endif
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly(true)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("dp::thread_pool coroutine array mult");
