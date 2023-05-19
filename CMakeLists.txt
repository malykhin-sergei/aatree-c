cmake_minimum_required(VERSION 3.5)

project(aatree LANGUAGES C)

include(CTest)

add_library(aatree SHARED aatree.c aatree_verify.c)
add_library(aatree-static STATIC aatree.c aatree_verify.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(aatree PRIVATE -O2 -g -Wall -Wextra -std=c89 -pedantic)

# Unit tests
enable_testing()

add_executable(aatree-unit-tests unit_tests.c)
add_test(unit-tests aatree-unit-tests)

target_link_libraries(aatree-unit-tests aatree)
target_compile_options(aatree-unit-tests PUBLIC -g -O0)
