cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ThreadPoolBenchmarks LANGUAGES CXX)

# --- Import tools ----
include(../cmake/tools.cmake)

# ---- Dependencies ----
include(../cmake/CPM.cmake)

CPMAddPackage(
    NAME benchmark
    GITHUB_REPOSITORY google/benchmark
    VERSION 1.6.1
    OPTIONS 
        "BENCHMARK_ENABLE_TESTING OFF"
        "CMAKE_CROSSCOMPILING ON"
        "HAVE_STD_REGEX OFF"
        "HAVE_POSIX_REGEX OFF"
)

CPMAddPackage(
    NAME cppcoro
    GITHUB_REPOSITORY andreasbuhr/cppcoro
    GIT_TAG 10bbcdbf2be3ad3aa56febcf4c7662d771460a99
)

if(benchmark_ADDED)
    # patch benchmark target
    set_target_properties(benchmark PROPERTIES CXX_STANDARD 20)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_executable(${PROJECT_NAME} ${headers} ${sources})
target_link_libraries(${PROJECT_NAME} benchmark ThreadPool::ThreadPool cppcoro)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(ThreadPool INTERFACE -Wall -Wpedantic -Wextra -Werror)
    elseif(MSVC)
        target_compile_options(ThreadPool INTERFACE /W4 /WX)
        target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
    endif()
endif()
