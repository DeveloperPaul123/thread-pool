/// Much of the code here is adopted from https://github.com/sol-prog/Mandelbrot_set
#pragma once

#include <array>
#include <complex>
#include <concepts>
#include <functional>
#include <mutex>
#include <span>
#include <vector>

// Use an alias to simplify the use of complex type
using complex = std::complex<double>;

struct rgb {
    int r{};
    int g{};
    int b{};
};

/// @brief Simple range class for viewing window and fractal window
template <class T>
requires std::integral<T> || std::floating_point<T>
struct fractal_window {
    T x_min{};
    T x_max{};
    T y_min{};
    T y_max{};
    constexpr T height() const noexcept { return y_max - y_min; }
    constexpr T width() const noexcept { return x_max - x_min; }
    constexpr T size() const noexcept { return width() * height(); }
};

/// @brief Performs smooth polynomial fitting to the given value.
rgb get_rgb_smooth(int n, int iter_max);

void save_ppm(fractal_window<int> &source, std::span<rgb> colors, std::string_view file_name);
/**
 * @brief Convert a pixel coordinate to the complex domain
 * @param scr Source domain (viewing window)
 * @param fr Fractal domain (real and imaginary range)
 * @param c Initial complex value
 */
complex scale(const fractal_window<int> &scr, const fractal_window<double> &fr, complex c);

/**
 * @brief Check if a point is in the set or escapes to infinity, return the number if iterations
 * @param c Complex number value, see scale
 * @param iter_max Max number of iterations
 * @param func The complex function used for the fractal.
 */
int escape(complex c, int iter_max, const std::function<complex(complex, complex)> &func);

/**
 * @brief Calculate a single fractal row and returns it's color values.
 * @param row The row to calculate (0 -> y_max)
 * @param source_window The source viewing window for the fractal.
 * @param fractal_window The fractal domain for imaginary and real parts.
 * @param iter_max Max number of iterations
 * @param func The fractal function to use.
 * @return vector of rgb values calculated for the given row.
 */
std::vector<rgb> calculate_fractal_row(int row, fractal_window<int> source_window,
                                       fractal_window<double> fractal_window, int iter_max,
                                       std::function<complex(complex, complex)> func);