#pragma once

#include <nanobench.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <random>
#include <span>
#include <vector>

inline std::size_t index(std::size_t row, std::size_t col, std::size_t width) {
    return row * width + col;
}

inline void multiply_array(std::span<int const> a, std::span<int const> b, std::span<int> result) {
    const auto size = static_cast<std::size_t>(std::sqrt(a.size()));
    for (std::size_t r = 0; r < size; ++r) {
        for (std::size_t c = 0; c < size; ++c) {
            for (std::size_t k = 0; k < size; ++k) {
                result[index(r, k, size)] += a[index(r, c, size)] * b[index(c, k, size)];
            }
        }
    }
}

template <typename T>
using multiplication_pair = std::pair<std::vector<T>, std::vector<T>>;

template <typename T, typename Rng = ankerl::nanobench::Rng>
[[nodiscard]] std::vector<multiplication_pair<T>> generate_benchmark_data(
    const std::size_t& array_size, const std::size_t& number_of_multiplications) {
    static std::uniform_int_distribution<T> distribution(std::numeric_limits<T>::min(),
                                                         std::numeric_limits<T>::max());

    static std::random_device device{};
    Rng rng(device());

    std::vector<multiplication_pair<T>> computations;
    computations.reserve(number_of_multiplications);

    const auto total_size = array_size * array_size;
    for (std::size_t i = 0; i < number_of_multiplications; ++i) {
        multiplication_pair<T> pr{};
        pr.first.reserve(total_size);
        pr.second.reserve(total_size);

        std::generate_n(std::back_inserter(pr.first), array_size * array_size,
                        [&rng] { return distribution(rng); });
        std::generate_n(std::back_inserter(pr.second), array_size * array_size,
                        [&rng] { return distribution(rng); });

        computations.push_back(std::move(pr));
    }

    return computations;
}

template <std::ranges::range Seq, typename ValueType = std::ranges::range_value_t<Seq>>
    requires std::is_integral_v<ValueType>
void generate_random_data(Seq&& seq) {
    static ankerl::nanobench::Rng rng(std::random_device{}());
    std::uniform_int_distribution<ValueType> distribution(std::numeric_limits<ValueType>::min(),
                                                          std::numeric_limits<ValueType>::max());
    std::ranges::generate(seq, [&] { return distribution(rng); });
}
