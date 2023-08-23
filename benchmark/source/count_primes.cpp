#include <doctest/doctest.h>
#include <nanobench.h>
#include <thread_pool/thread_pool.h>
#include <utilities.h>

#include <BS_thread_pool_light.hpp>
#include <iostream>
#include <random>
#include <riften/thiefpool.hpp>

template <std::integral ValueType>
bool is_prime(const ValueType& value) {
    if (value <= 3 && value > 1) return true;

    // no need to check above sqrt(n)
    const auto n = static_cast<ValueType>(std::ceil(std::sqrt(value) + 1));

    for (auto i = 2; i < n; ++i) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

template <std::integral ValueType>
void count_if_prime(const ValueType& value, std::uint64_t& count) {
    if (is_prime(value)) ++count;
}

template <std::integral ValueType>
void count_if_prime_tp(const ValueType& value, std::atomic<std::uint64_t>& count) {
    if (is_prime(value)) ++count;
}

template <std::integral ValueType>
std::uint64_t count_primes(const std::vector<ValueType>& values) {
    std::uint64_t count = 0;
    for (const auto& value : values) count_if_prime(value, std::ref(count));
    return count;
}

template <std::integral ValueType>
std::uint64_t count_primes_thread_pool(const std::vector<ValueType>& values) {
    std::atomic<std::uint64_t> count(0);

    {
        dp::thread_pool<> pool{};
        for (const auto& value : values) {
            pool.enqueue_detach(count_if_prime_tp<std::uint64_t>, value, std::ref(count));
        }
    }

    return count.load();
}

template <std::integral ValueType>
void run_benchmark(const std::size_t& size) {
    ankerl::nanobench::Bench bench;
    auto bench_title = std::string("count primes ") + std::to_string(sizeof(ValueType) * 8) +
                       " bit " + std::to_string(size);
    bench.title(bench_title).warmup(10).relative(true);

    // generate the data
    std::vector<ValueType> values(size);
    generate_random_data(values);

    std::atomic<std::uint64_t> count(0);
    bench.run("dp::thread_pool", [&] {
        {
            dp::thread_pool<> pool{};
            for (const auto& value : values) {
                pool.enqueue_detach(count_if_prime_tp<ValueType>, value, std::ref(count));
            }
        }
    });

    count.store(0);
    bench.run("BS::thread_pool_light", [&] {
        BS::thread_pool_light bs_thread_pool{std::thread::hardware_concurrency()};
        for (const auto& value : values) {
            bs_thread_pool.push_task(count_if_prime_tp<ValueType>, value, std::ref(count));
        }
    });

    count.store(0);
    bench.run("riften::thief_pool", [&] {
        riften::Thiefpool pool{};
        for (const auto& value : values) {
            pool.enqueue_detach(count_if_prime_tp<ValueType>, value, std::ref(count));
        }
    });
}

TEST_CASE("count primes") {
    using namespace std::chrono_literals;

    // test sequentially and with thread pool
    std::vector<std::uint64_t> values(100);
    generate_random_data(values);

    auto result = count_primes(values);
    auto pool_result = count_primes_thread_pool(values);

    CHECK(result == pool_result);

    std::vector<std::uint32_t> values2(100);
    generate_random_data(values2);

    result = count_primes(values2);
    pool_result = count_primes_thread_pool(values2);

    CHECK(result == pool_result);

    std::vector<std::uint16_t> values3(100);
    generate_random_data(values3);

    result = count_primes(values3);
    pool_result = count_primes_thread_pool(values3);

    CHECK(result == pool_result);

    std::vector<std::size_t> small_int_args = {10'000, 100'000, 1'000'000};
    for (const auto& size : small_int_args) {
        run_benchmark<std::uint16_t>(size);
    }

    std::vector<std::size_t> args = {100, 1000, 10'000};
    for (const auto& size : args) {
        run_benchmark<std::uint32_t>(size);
    }

    std::vector<std::size_t> large_int_args = {100, 1000};
    for (const auto& size : large_int_args) {
        run_benchmark<std::uint64_t>(size);
    }
}
