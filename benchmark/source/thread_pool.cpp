#include <benchmark/benchmark.h>
#include <thread_pool/thread_pool.h>

#include "utilities.h"

static void BM_array_multiplication(benchmark::State& state) {
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

BENCHMARK(BM_array_multiplication)
    ->Args({8, 25'000})
    ->Args({64, 5'000})
    ->Args({256, 250})
    ->Args({512, 75})
    ->Args({1024, 10})
    ->Unit(benchmark::kMillisecond)
    ->ReportAggregatesOnly(true)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("dp::thread_pool array mult");
