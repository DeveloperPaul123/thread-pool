#include <benchmark/benchmark.h>

#include <future>

#include "utilities.h"

static void BM_array_multiplication_std_async(benchmark::State& state) {
    const auto array_size = state.range(0);
    const std::size_t multiplications_to_perform = state.range(1);

    // generate test data
    const auto computations = generate_benchmark_data<int>(array_size, multiplications_to_perform);

    // task that runs on a new thread (potentially)
    auto thread_task = [](multiplication_pair<int> pair) {
        std::vector<int> result(pair.first.size());
        multiply_array(pair.first, pair.second, result);
    };

    // set up vector for results
    std::vector<std::future<void>> results;
    results.reserve(computations.size());

    for (auto _ : state) {
        
        for (const auto& mult : computations) {
            // let std async decide on how to launch the task, either deferred or async
            results.emplace_back(std::async(thread_task, mult));
        }

        // wait for futures
        for (auto& fut : results) {
            fut.wait();
        }
    }
}

BENCHMARK(BM_array_multiplication_std_async)
    ->Args({8, 25'000})
    ->Args({64, 5'000})
    ->Args({256, 250})
    ->Args({512, 75})
    ->Args({1024, 10})
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly(true)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("std::async array mult");
