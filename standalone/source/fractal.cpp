#include "fractal.h"

#include <iostream>
#include <fstream>
#include <chrono>

std::array<int, 3> get_rgb_smooth(int n, int iter_max) {
  // map n on the 0..1 interval
  double t = (double)n / (double)iter_max;

  // Use smooth polynomials for r, g, b
  int r = (int)(9 * (1 - t) * t * t * t * 255);
  int g = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
  int b = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
  return {r, g, b};
}

void plot(fractal_window<int>& scr, std::vector<int>& colors, int iter_max, const char* fname) {
  constexpr auto new_line = '\n';

  unsigned int width = scr.width(), height = scr.height();
  std::ofstream output_ppm(fname, std::ios::binary | std::ios::out);
  // TODO: Switch to using std::format
  // output header
  output_ppm << "P3 " << width << " " << height << " " << 255 << new_line;
  std::array<int, 3> rgb;
  for (int i = scr.y_min; i < scr.y_max; ++i) {
    for (int j = scr.x_min; j < scr.x_max; ++j) {
      auto index = i * width + j;
      int n = colors[index];
      rgb = get_rgb_smooth(n, iter_max);
      output_ppm << std::get<0>(rgb) << " " << std::get<1>(rgb) << " " << std::get<2>(rgb) << " ";
      // TODO: Output to file
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

std::vector<int> calculate_fractal_row(int row, fractal_window<int> source_window,
                                       fractal_window<double> fractal_window, int iter_max,
                                       std::function<complex(complex, complex)> func) {
  std::vector<int> fract_row(source_window.width());
  for (int col = source_window.x_min; col < source_window.x_max; ++col) {
    auto index = col + source_window.x_min;
    complex comp((double)col, (double)row);
    comp = scale(source_window, fractal_window, comp);
    fract_row[index] = escape(comp, iter_max, func);
  }

  return fract_row;
}

// Loop over each pixel from our image and check if the points associated with this pixel escape
// to infinity
void get_number_iterations(fractal_window<int>& scr, fractal_window<double>& fract, int iter_max,
                           std::vector<int>& colors,
                           const std::function<complex(complex, complex)>& func) {
  int k = 0, progress = -1;
  for (int i = scr.y_min; i < scr.y_max; ++i) {
    for (int j = scr.x_min; j < scr.x_max; ++j) {
      complex c((double)j, (double)i);
      c = scale(scr, fract, c);
      colors[k] = escape(c, iter_max, func);
      k++;
    }
    if (progress < (int)(i * 100.0 / scr.y_max)) {
      progress = (int)(i * 100.0 / scr.y_max);
      std::cout << progress << "%\n";
    }
  }
}

void fractal(fractal_window<int>& scr, fractal_window<double>& fract, int iter_max,
             std::vector<int>& colors, const std::function<complex(complex, complex)>& func,
             const char* fname) {
  auto start = std::chrono::steady_clock::now();
  get_number_iterations(scr, fract, iter_max, colors, func);
  auto end = std::chrono::steady_clock::now();
  std::cout << "Time to generate " << fname << " = "
            << std::chrono::duration<double, std::milli>(end - start).count() << " [ms]"
            << std::endl;

  // Save (show) the result as an image
  plot(scr, colors, iter_max, fname);
}
