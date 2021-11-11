#include <thread_pool/thread_pool.h>

#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "fractal.h"

void mandelbrot_threadpool(int image_width, int image_height, int max_iterations,
                           std::string_view output_file_name) {
    fractal_window<int> source{0, image_width, 0, image_height};
    fractal_window<double> fract{-2.2, 1.2, -1.7, 1.7};

    auto complex_func = [](complex z, complex c) -> complex { return z * z + c; };

    std::cout << "calculating mandelbrot" << std::endl;

    dp::thread_pool pool;
    std::vector<std::future<std::vector<rgb>>> futures;
    for (auto row = 0; row < source.height(); row++) {
        auto task = [task_row = row](
                        fractal_window<int> source_window, fractal_window<double> fractal_window,
                        int iter_max,
                        std::function<complex(complex, complex)> func) -> std::vector<rgb> {
            return calculate_fractal_row(task_row, source_window, fractal_window, iter_max, func);
        };

        futures.push_back(pool.enqueue(task, source, fract, max_iterations, complex_func));
    }

    // reserve memory for output rgb
    std::vector<rgb> colors;
    colors.reserve(source.size());

    // copy data to output vector
    for (auto &future : futures) {
        auto data = future.get();
        colors.insert(colors.end(), data.begin(), data.end());
    }

    std::cout << "mandelbrot completed" << std::endl;
    std::cout << "saving results..." << std::endl;
    // save result
    save_ppm(source, colors, output_file_name);
}

auto main(int argc, char **argv) -> int {
    cxxopts::Options options(*argv, "Generate a mandelbrot ppm image using a threadpool!");

    int image_width;
    int image_height;
    int max_iterations;
    std::string output_file_name;
    // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("x,width", "Image width", cxxopts::value(image_width)->default_value("2000"))
    ("y,height", "Image height", cxxopts::value(image_height)->default_value("2000"))
    ("n,iterations", "Max iterations", cxxopts::value(max_iterations)->default_value("30"))
    ("o,filename", "Output file name", cxxopts::value(output_file_name)->default_value("mandelbrot.ppm"))
  ;
    // clang-format on

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help({"", "Group"}) << std::endl;
            exit(0);
        }

        mandelbrot_threadpool(image_width, image_height, max_iterations, output_file_name);

    } catch (const cxxopts::OptionException &e) {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
