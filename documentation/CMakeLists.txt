cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ThreadPoolDocs)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:jothepro/doxygen-awesome-css#v1.6.1")
find_package(Doxygen REQUIRED)

# ---- Doxygen variables ----

# set Doxyfile variables
set(DOXYGEN_PROJECT_NAME thread-pool)
set(DOXYGEN_PROJECT_VERSION ${thread-pool_VERSION})
set(DOXYGEN_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

configure_file(${CMAKE_CURRENT_LIST_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_custom_target(
    GenerateDocs
    ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
    COMMAND Doxygen::doxygen ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    COMMAND echo "Docs written to: ${DOXYGEN_OUTPUT_DIRECTORY}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
