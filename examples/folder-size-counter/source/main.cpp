#include <thread_pool/thread_pool.h>

#include <cxxopts.hpp>
#include <filesystem>
#include <vector>

std::uintmax_t folder_size(const std::string &path) {
  std::vector<std::future<std::uintmax_t>> futures;
  dp::thread_pool pool;
  std::uintmax_t total_count{};
  for (auto file : std::filesystem::directory_iterator(path)) {
    std::cout << file << std::endl;
    if (file.is_directory()) {
      futures.push_back(pool.enqueue([thread_folder = file]() -> std::uintmax_t {
        std::uintmax_t count{};
        for (const auto &file : std::filesystem::recursive_directory_iterator(thread_folder)) {
          try {
            count += file.file_size();
          } catch (std::filesystem::filesystem_error &error) {
            std::cout << "error: " << error.code() << " " << error.what() << "\n\t"
                      << file.path().string() << "\n";
          }
        }
        return count;
      }));
    } else {
      total_count += file.file_size();
    }
  }

  
  for (auto &future : futures) {
    total_count += future.get();
  }

  return total_count;
}

auto main(int argc, char **argv) -> int {
  cxxopts::Options options(*argv, "Count the size of a folder");

  std::string folder_path;
  // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("p,path", "Folder path", cxxopts::value(folder_path)->default_value("E:/Videos"))
  ;
  // clang-format on

  try {
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }

    const auto size = folder_size(folder_path);
    std::cout << "Size: " << size << " bytes" << std::endl;
  } catch (const cxxopts::OptionException &e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
