cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

# ---- Project ----

# Note: update this to your new project's name and version
project(
    thread-pool
    VERSION 0.6.2
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

# add source files
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

find_package(Threads REQUIRED)

# ---- Create library ----
add_library(${PROJECT_NAME} INTERFACE)
add_library(dp::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)

if(CMAKE_CXX_STANDARD GREATER 20)
    target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
else()
    target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
endif()

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->)

# the location where the project's version header will be placed should match the project's regular
# header paths here we get the root folder of the include directory but getting the directory name
# from one of the headers paths
list(GET headers 0 first_header)
get_filename_component(include_dir_path ${first_header} DIRECTORY)
get_filename_component(include_dir_name ${include_dir_path} NAME)

configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/version.h.in ${PROJECT_BINARY_DIR}/include/thread_pool/version.h
    @ONLY
)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
              $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
              $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Installation help
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}"
)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake" COMPATIBILITY SameMajorVersion
                                                                               ARCH_INDEPENDENT
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets)

install(
    EXPORT ${PROJECT_NAME}-targets
    NAMESPACE dp::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}"
    FILE "${PROJECT_NAME}-targets.cmake"
)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}
)

install(FILES ${headers}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${PROJECT_VERSION}/thread_pool
)
install(FILES ${PROJECT_BINARY_DIR}/include/thread_pool/version.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${PROJECT_VERSION}/thread_pool
)

option(TP_BUILD_TESTS "Turn on to build unit tests." ON)
option(TP_BUILD_EXAMPLES "Turn on to build examples." ON)
option(TP_BUILD_BENCHMARKS "Turn on to build benchmarks." ON)

if(${TP_BUILD_TESTS} OR ${TP_BUILD_EXAMPLES} OR ${TP_BUILD_BENCHMARKS})
    # see https://github.com/TheLartians/CPM.cmake for more info
    include(cmake/CPM.cmake)
endif()

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
