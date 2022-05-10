#include <benchmark/benchmark.h>

#include <array>

#include "utilities.h"

int main(int argc, char** argv) {
    // quick test for matrix multiplication
    std::array a{3, 7, 4, 9};
    std::array b{6, 2, 5, 8};
    std::array<int, 4> c{};

    multiply_array(a, b, c);
    assert(c[0] == 53);
    assert(c[1] == 62);
    assert(c[2] == 69);
    assert(c[3] == 80);

    // set up all the benchmarks and run them
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
