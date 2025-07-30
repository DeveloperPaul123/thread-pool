#include <thread_pool/thread_pool.h>
#include <thread_pool/version.h>

#include <chrono>
#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "fractal.h"

void mandelbrot_threadpool(int image_width, int image_height, int max_iterations,
                           std::string_view output_file_name) {
    const fractal_window<int> source{0, image_width, 0, image_height};
    const fractal_window<double> fract{-2.2, 1.2, -1.7, 1.7};

    auto complex_func = [](complex z, complex c) -> complex { return z * z + c; };

    std::cout << "calculating mandelbrot" << std::endl;

    dp::thread_pool pool;
    std::vector<std::future<std::vector<rgb>>> futures;
    futures.reserve(source.height());
    const auto start = std::chrono::steady_clock::now();

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

    const auto end = std::chrono::steady_clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "mandelbrot completed: " << duration << std::endl;
    std::cout << "saving results..." << std::endl;
    // save result
    save_ppm(source.width(), source.height(), colors, output_file_name);
}

auto main(int argc, char **argv) -> int {
    cxxopts::Options options(*argv, "Generate a mandelbrot ppm image using a thread pool!");

    int image_size;
    int max_iterations;
    std::string output_file_name;
    // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("s,size", "Image size", cxxopts::value(image_size)->default_value("2000"))
    ("n,iterations", "Max iterations", cxxopts::value(max_iterations)->default_value("30"))
    ("o,filename", "Output file name", cxxopts::value(output_file_name)->default_value("mandelbrot.ppm"))
  ;
    // clang-format on

    try {
        const auto result = options.parse(argc, argv);

        std::cout << "Using dp::thread-pool version " << THREADPOOL_VERSION << '\n';

        if (result.count("help")) {
            std::cout << options.help({"", "Group"}) << std::endl;
            exit(0);
        }

        mandelbrot_threadpool(image_size, image_size, max_iterations, output_file_name);

    } catch (const cxxopts::exceptions::exception &e) {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
