#include <thread_pool/thread_pool.h>

#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

auto main(int argc, char** argv) -> int {
  cxxopts::Options options(*argv, "A program to welcome the world!");

  std::string language;
  std::string name;

  // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("v,version", "Print the current version number")
    ("n,name", "Name to greet", cxxopts::value(name)->default_value("World"))
    ("l,lang", "Language code to use", cxxopts::value(language)->default_value("en"))
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  dp::thread_pool pool();
  pool.enqueue_detach([](){ std::cout << "Hi" << std::endl;});
  pool.enqueue_detach([](){ std::cout << "Hi" << std::endl;});
  pool.enqueue_detach([](){ std::cout << "Hi" << std::endl;});
  pool.enqueue_detach([](){ std::cout << "Hi" << std::endl;});
  pool.enqueue_detach([](){ std::cout << "Hi" << std::endl;});
  
  return 0;
}
