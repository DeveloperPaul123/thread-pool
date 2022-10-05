#pragma once

#include <algorithm>
#include <cmath>
#include <cppcoro/task.hpp>
#include <iterator>
#include <random>
#include <span>
#include <vector>

#include "thread_pool/thread_pool.h"

inline std::size_t index(std::size_t row, std::size_t col, std::size_t width) {
    return row * width + col;
}

inline void multiply_array(std::span<int> a, std::span<int> b, std::span<int> result) {
    const auto size = static_cast<std::size_t>(std::sqrt(a.size()));
    for (std::size_t r = 0; r < size; ++r) {
        for (std::size_t c = 0; c < size; ++c) {
            for (std::size_t k = 0; k < size; ++k) {
                result[index(r, k, size)] += a[index(r, c, size)] * b[index(c, k, size)];
            }
        }
    }
}

inline cppcoro::task<> co_multiply_array(std::span<int> a, std::span<int> b, std::span<int> result,
                                         dp::thread_pool<>& pool) {
    co_await pool.schedule();
    multiply_array(a, b, result);
}

template <typename T>
using multiplication_pair = std::pair<std::vector<T>, std::vector<T>>;

template <typename T>
[[nodiscard]] std::vector<multiplication_pair<T>> generate_benchmark_data(
    const std::int64_t& array_size, const std::int64_t& number_of_multiplications) {
    static std::uniform_int_distribution<T> distribution(std::numeric_limits<T>::min(),
                                                         std::numeric_limits<T>::max());
    // yes, predictable values
    static std::default_random_engine generator{};

    std::vector<multiplication_pair<T>> computations;
    computations.reserve(number_of_multiplications);

    const auto total_size = array_size * array_size;
    for (auto i = 0; i < number_of_multiplications; ++i) {
        multiplication_pair<T> pr{};
        pr.first.reserve(total_size);
        pr.second.reserve(total_size);

        std::generate_n(std::back_inserter(pr.first), array_size * array_size,
                        []() { return distribution(generator); });
        std::generate_n(std::back_inserter(pr.second), array_size * array_size,
                        []() { return distribution(generator); });

        computations.push_back(std::move(pr));
    }

    return computations;
}
