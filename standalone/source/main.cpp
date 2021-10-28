#include <thread_pool/thread_pool.h>

#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "fractal.h"

struct fractal_row {
  std::vector<rgb> data{};
  int row{0};
};

void mandelbrot_threadpool() {
  fractal_window<int> source{0, 5000, 0, 5000};
  fractal_window<double> fract{-2.2, 1.2, -1.7, 1.7};

  auto complex_func = [](complex z, complex c) -> complex { return z * z + c; };

  int iter_max = 1000;

  std::cout << "calculating mandelbrot" << std::endl;

  dp::thread_pool pool;
  std::vector<std::future<fractal_row>> futures;
  for (auto row = 0; row < source.height(); row++) {
    auto task = [task_row = row](fractal_window<int> source_window,
                                 fractal_window<double> fractal_window, int iter_max,
                                 std::function<complex(complex, complex)> func) -> fractal_row {
      fractal_row fract_row{};
      fract_row.row = task_row;

      fract_row.data
          = calculate_fractal_row(task_row, source_window, fractal_window, iter_max, func);
      return fract_row;
    };

    futures.push_back(pool.enqueue(task, source, fract, iter_max, complex_func));
  }

  // reserve memory for output rgb
  std::vector<rgb> colors;
  colors.reserve(source.size());

  // copy data to output vector
  for (auto &future : futures) {
    auto row_info = future.get();
    auto data = row_info.data;
    colors.insert(colors.end(), data.begin(), data.end());
  }

  std::cout << "mandelbrot completed" << std::endl;
  std::cout << "saving results..." << std::endl;
  // save result
  save_ppm(source, colors, "mandelbrot_mt.ppm");
}

auto main(int argc, char **argv) -> int {
  mandelbrot_threadpool();

  cxxopts::Options options(*argv, "Under construction");

  int image_width;
  int image_height;

  // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("v,version", "Print the current version number")
    ("w,width", "Image width", cxxopts::value(image_width)->default_value("1080"))
    ("y,height", "Image height", cxxopts::value(image_height)->default_value("1920"))
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  return 0;
}
