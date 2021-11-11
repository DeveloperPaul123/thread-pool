#include "fractal.h"

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

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
    std::ofstream output_ppm(file_name.data(), std::ios::binary | std::ios::out);
    // TODO: Switch to using std::format
    // output header
    output_ppm << "P3 " << width << " " << height << " " << 255 << new_line;
    for (int i = source.y_min; i < source.y_max; ++i) {
        for (int j = source.x_min; j < source.x_max; ++j) {
            auto index = i * width + j;
            auto pixel = colors[index];
            output_ppm << pixel.r << " " << pixel.g << " " << pixel.b << " ";
        }
        output_ppm << new_line;
    }
}

void plot(fractal_window<int>& scr, std::vector<int>& colors, int iter_max, const char* fname) {
    constexpr auto new_line = '\n';

    unsigned int width = scr.width(), height = scr.height();
    std::ofstream output_ppm(fname, std::ios::binary | std::ios::out);
    // TODO: Switch to using std::format
    // output header
    output_ppm << "P3 " << width << " " << height << " " << 255 << new_line;
    for (int i = scr.y_min; i < scr.y_max; ++i) {
        for (int j = scr.x_min; j < scr.x_max; ++j) {
            auto index = i * width + j;
            int n = colors[index];
            auto pixel = get_rgb_smooth(n, iter_max);
            output_ppm << pixel.r << " " << pixel.g << " " << pixel.b << " ";
        }
        output_ppm << new_line;
    }
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
