cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(thread-pool-tests LANGUAGES CXX)

# ---- Options ----
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:doctest/doctest@2.4.11")
CPMAddPackage(
  NAME Format.cmake
  VERSION 1.7.3
  GITHUB_REPOSITORY TheLartians/Format.cmake
  OPTIONS 
      # skip CMake formatting for now
      "FORMAT_SKIP_CMAKE YES"
      # path to exclude (optional, supports regular expressions)
      "CMAKE_FORMAT_EXCLUDE ${CMAKE_SOURCE_DIR}/cmake/CPM.cmake"
)

if(TEST_INSTALLED_VERSION)
    # testing the installed version, so we assume this is the top level CMakeLists.txt
    enable_testing()
    find_package(thread-pool REQUIRED)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} doctest::doctest dp::thread-pool)

target_compile_options(
    ${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU>:--coverage>
)

target_link_options(
    ${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU>:--coverage>
)

if(NOT WIN32)
    target_compile_options(
            ${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fsanitize=thread -g>
    )
    target_link_options(
            ${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fsanitize=thread>
    )
elseif(MSVC AND ${CMAKE_BUILD_TYPE} MATCHES "Debug;RelWithDebInfo")
    target_compile_options(
            ${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/fsanitize=address>
    )
endif()

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
    target_compile_options(
        ${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang,GNU>:-Wall -Wpedantic -Wextra
                             -Werror>
    )
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /wd4324>)
    target_compile_definitions(
        ${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:DOCTEST_CONFIG_USE_STD_HEADERS>
    )
endif()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})
