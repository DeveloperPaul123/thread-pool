cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

# Note: update this to your new project's name and version
project(
    ThreadPool
    VERSION 0.4.1
    LANGUAGES CXX
)

message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

# Option to override which C++ standard to use
set(TP_CXX_STANDARD
    DETECT
    CACHE STRING "Override the default CXX_STANDARD to compile with."
)
set_property(CACHE TP_CXX_STANDARD PROPERTY STRINGS DETECT 20 23)

# Decide on the standard to use
if(TP_CXX_STANDARD STREQUAL "20")
    if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        message(STATUS "Using C++20 standard")
        set(CMAKE_CXX_STANDARD 20)
    else()
        message(
            FATAL_ERROR "Requested TP_CXX_STANDARD \"20\" not supported by provided C++ compiler"
        )
    endif()
elseif(TP_CXX_STANDARD STREQUAL "23")
    if("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        message(STATUS "Using C++23 standard")
        set(CMAKE_CXX_STANDARD 23)
    else()
        message(
            FATAL_ERROR "Requested TP_CXX_STANDARD \"23\" not supported by provided C++ compiler"
        )
    endif()
else()
    if("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        set(CMAKE_CXX_STANDARD 23)
        message(STATUS "Detected support for C++23 standard")
    elseif("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        set(CMAKE_CXX_STANDARD 20)
        message(STATUS "Detected support for C++20 standard")
    else()
        message(FATAL_ERROR "Cannot detect CXX_STANDARD of C++20 or newer.")
    endif()
endif()

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info
include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.6.0")

# ---- Add source files ----
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

find_package(Threads REQUIRED)

# ---- Create library ----
add_library(${PROJECT_NAME} INTERFACE)
add_library(dp::thread-pool ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)

if(CMAKE_CXX_STANDARD GREATER 20)
    target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
else()
    target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
endif()

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->)

target_include_directories(
    ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                              $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths here we get the root folder of the include directory but getting the directory name
# from one of the headers paths
list(GET headers 0 first_header)
get_filename_component(include_dir_path ${first_header} DIRECTORY)
get_filename_component(include_dir_name ${include_dir_path} NAME)
string(TOLOWER ${include_dir_name}/version.h VERSION_HEADER_LOCATION)

packageProject(
    NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    NAMESPACE ${PROJECT_NAME}
    BINARY_DIR ${PROJECT_BINARY_DIR}
    INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
    INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
    VERSION_HEADER "${VERSION_HEADER_LOCATION}"
    COMPATIBILITY SameMajorVersion
    DEPENDENCIES "Threads"
)

option(TP_BUILD_TESTS "Turn on to build unit tests." ON)
option(TP_BUILD_EXAMPLES "Turn on to build examples." ON)
option(TP_BUILD_BENCHMARKS "Turn on to build benchmarks." ON)

if(${TP_BUILD_TESTS})
    enable_testing()
    add_subdirectory(test)
endif()
if(${TP_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()
if(${TP_BUILD_BENCHMARKS})
    add_subdirectory(benchmark)
endif()
