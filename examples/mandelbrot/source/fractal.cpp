#include "fractal.h"

#include <chrono>
#include <fstream>

rgb get_rgb_smooth(int n, int iter_max) {
    // map n on the 0..1 interval
    double t = (double)n / (double)iter_max;

    // Use smooth polynomials for r, g, b
    int r = (int)(9 * (1 - t) * t * t * t * 255);
    int g = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
    int b = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return {r, g, b};
}

void save_ppm(fractal_window<int>& source, std::span<rgb> colors, std::string_view file_name) {
    constexpr auto new_line = '\n';

    unsigned int width = source.width(), height = source.height();
    std::ofstream output_ppm(file_name.data(), std::ios::binary);
    // TODO: Switch to using std::format
    // output header
    output_ppm << "P6 " << new_line;
    output_ppm << "# created by DeveloperPaul123/thread-pool mandelbrot sample.\n";
    output_ppm << width << " " << height << " " << 255 << new_line;

    auto buffer = std::make_unique<uint8_t[]>(width * height * 3);

    for (int i = source.y_min; i < source.y_max; ++i) {
        for (int j = source.x_min; j < source.x_max; ++j) {
            const auto index = i * width + j;
            const auto pixel = colors[index];
            const auto buffer_index = i * width * 3 + (j + source.x_min) * 3;

            buffer[buffer_index] = static_cast<uint8_t>(pixel.r);
            buffer[buffer_index + 1] = static_cast<uint8_t>(pixel.g);
            buffer[buffer_index + 2] = static_cast<uint8_t>(pixel.b);
        }
    }

    output_ppm.write(reinterpret_cast<char*>(buffer.get()), width * height * 3);
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
        auto index = col - source_window.x_min;
        complex comp((double)col, (double)row);
        comp = scale(source_window, fractal_window, comp);
        auto value = escape(comp, iter_max, func);
        auto rgb = get_rgb_smooth(value, iter_max);
        output[index] = rgb;
    }
    return output;
}
