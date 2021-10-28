/// Much of the code here is adopted from https://github.com/sol-prog/Mandelbrot_set
#pragma once

#include <array>
#include <complex>
#include <concepts>
#include <vector>
#include <functional>

// Use an alias to simplify the use of complex type
using complex = std::complex<double>;

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
std::array<int, 3> get_rgb_smooth(int n, int iter_max);

/// @brief Converts the colors vector to true RGB and saves the result to a .ppm image file.
void plot(fractal_window<int> &scr, std::vector<int> &colors, int iter_max, const char *fname);

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
 * @brief Loop over each pixel from our image and check if the points associated with this pixel escape to infinity
 * @param scr Source or viewing window.
 * @param fract Fractal domain for real and imaginary parts
 * @param iter_max Max number of iterations
 * @param colors vector that holds color value (converted to RGB later)
 * @param func Fractal function.
*/
void get_number_iterations(fractal_window<int> &scr, fractal_window<double> &fract, int iter_max,
                           std::vector<int> &colors,
                           const std::function<complex(complex, complex)> &func);

void fractal(fractal_window<int> &scr, fractal_window<double> &fract, int iter_max,
             std::vector<int> &colors, const std::function<complex(complex, complex)> &func,
             const char *fname);

/**
 * @brief Calculate a single fractal row and returns it's color values.
 * @param row The row to calculate (0 -> y_max)
 * @param source_window The source viewing window for the fractal.
 * @param fractal_window The fractal domain for imaginary and real parts.
 * @param iter_max Max number of iterations
 * @param func The fractal function to use.
 * @return Values calculated for the given row.
*/
std::vector<int> calculate_fractal_row(int row, fractal_window<int> source_window,
                                       fractal_window<double> fractal_window, int iter_max,
                                       std::function<complex(complex, complex)> func);