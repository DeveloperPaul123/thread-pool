#include "fractal.h"

#include <chrono>
#include <fstream>
#include <memory>

rgb get_rgb_smooth(int n, int iter_max) {
    // map n on the 0..1 interval
    double t = (double)n / (double)iter_max;

    // Use smooth polynomials for r, g, b
    auto r = (uint8_t)(9 * (1 - t) * t * t * t * 255);
    auto g = (uint8_t)(15 * (1 - t) * (1 - t) * t * t * 255);
    auto b = (uint8_t)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return {r, g, b};
}

void save_ppm(const unsigned int& width, const unsigned int& height, std::span<rgb> colors,
              std::string_view file_name) {
    constexpr auto new_line = '\n';

    std::ofstream output_ppm(file_name.data(), std::ios::binary);
    // output header
    output_ppm << "P6 " << new_line;
    output_ppm << "# created by DeveloperPaul123/thread-pool mandelbrot sample.\n";
    output_ppm << width << " " << height << " " << 255 << new_line;

    output_ppm.write(reinterpret_cast<char*>(colors.data()), width * height * 3);
}

// Convert a pixel coordinate to the complex domain
complex scale(const fractal_window<int>& scr, const fractal_window<double>& fr, complex c) {
    complex aux(c.real() / (double)scr.width() * fr.width() + fr.x_min,
                c.imag() / (double)scr.height() * fr.height() + fr.y_min);
    return aux;
}

// Check if a point is in the set or escapes to infinity, return the number if iterations
int escape(complex c, int iter_max, const std::function<complex(complex, complex)>& func) {
    complex z(0);
    int iter = 0;

    while (abs(z) < 2.0 && iter < iter_max) {
        z = func(z, c);
        iter++;
    }

    return iter;
}

std::vector<rgb> calculate_fractal_row(int row, fractal_window<int> source_window,
                                       fractal_window<double> fractal_window, int iter_max,
                                       std::function<complex(complex, complex)> func) {
    std::vector<rgb> output(source_window.width());
    for (int col = source_window.x_min; col < source_window.x_max; ++col) {
        const auto index = col - source_window.x_min;
        complex comp((col), (row));
        comp = scale(source_window, fractal_window, comp);
        const auto value = escape(comp, iter_max, func);
        const auto rgb = get_rgb_smooth(value, iter_max);
        output[index] = rgb;
    }
    return output;
}
