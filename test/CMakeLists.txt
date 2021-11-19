cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ThreadPoolTests LANGUAGES CXX)

# ---- Options ----
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:onqtam/doctest#2.4.5")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.0")

if(TEST_INSTALLED_VERSION)
    find_package(ThreadPool REQUIRED)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} doctest::doctest ThreadPool::ThreadPool)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(ThreadPool INTERFACE -Wall -Wpedantic -Wextra -Werror)
    elseif(MSVC)
        target_compile_options(ThreadPool INTERFACE /W4 /WX)
        target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
    endif()
endif()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})
