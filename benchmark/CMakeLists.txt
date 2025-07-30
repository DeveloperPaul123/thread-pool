cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(thread-pool-benchmarks LANGUAGES CXX)

# --- Import tools ----
include(../cmake/tools.cmake)

# ---- Dependencies ----
include(../cmake/CPM.cmake)

CPMAddPackage(
    NAME nanobench
    GITHUB_REPOSITORY martinus/nanobench
    VERSION 4.3.11
    GIT_SHALLOW
)

CPMAddPackage("gh:doctest/doctest@2.4.12")
CPMAddPackage(
    NAME bshoshany
    GITHUB_REPOSITORY bshoshany/thread-pool
    VERSION 3.5.0
    GIT_SHALLOW
)

if(bshoshany_ADDED)
    add_library(bshoshany INTERFACE IMPORTED)
    target_include_directories(bshoshany INTERFACE ${bshoshany_SOURCE_DIR}/include)
endif()

CPMAddPackage(
    NAME ConorWilliams_ThreadPool
    GITHUB_REPOSITORY ConorWilliams/Threadpool
    VERSION 2.1.1
    GIT_TAG a757265d75c189ad0fd0e429c166efb6281f2770
    GIT_SHALLOW
)

CPMAddPackage(
        NAME task-thread-pool
        GITHUB_REPOSITORY alugowski/task-thread-pool
        VERSION 1.0.6
        GIT_SHALLOW
)

# ---- Create binary ----
file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_executable(${PROJECT_NAME} ${headers} ${sources})
target_link_libraries(${PROJECT_NAME} nanobench doctest::doctest bshoshany RiftenThiefpool dp::thread-pool task-thread-pool::task-thread-pool)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wpedantic -Wextra)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} INTERFACE /W4)
    target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:DOCTEST_CONFIG_USE_STD_HEADERS>)
endif()

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} compiler_id)

set(results_markdown_file "${CMAKE_CURRENT_SOURCE_DIR}/results/benchmark_results_${compiler_id}.md")
target_compile_definitions(${PROJECT_NAME} PUBLIC RESULTS_MARKDOWN_FILE="${results_markdown_file}")