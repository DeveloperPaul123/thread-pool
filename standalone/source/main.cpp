#include <thread_pool/thread_pool.h>

#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>

auto main(int argc, char** argv) -> int {
  cxxopts::Options options(*argv, "A program to welcome the world!");

  int image_width;
  int image_height;

  // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("v,version", "Print the current version number")
    ("w,width", "Image width", cxxopts::value(image_width)->default_value("1080"))
    ("h,height", "Image height", cxxopts::value(image_height)->default_value("1920"))
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  dp::thread_pool pool(4);
  pool.enqueue_detach([]() { std::cout << "Hi from " << std::this_thread::get_id() << std::endl; });
  pool.enqueue_detach([]() { std::cout << "Hi from " << std::this_thread::get_id() << std::endl; });
  pool.enqueue_detach([]() { std::cout << "Hi from " << std::this_thread::get_id() << std::endl; });
  pool.enqueue_detach([]() { std::cout << "Hi from " << std::this_thread::get_id() << std::endl; });
 
  return 0;
}
