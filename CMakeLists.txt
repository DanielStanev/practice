# Optional CMake build of every problem set (each set also builds on its own,
# and the per-problem run_tests.sh scripts need nothing but a compiler).
#
#   cmake -S . -B build                 # ASan + UBSan for all sets
#   cmake --build build -j
#   ctest --test-dir build --output-on-failure      # everything
#   ctest --test-dir build -R 09_knapsack           # one problem
#
# Sanitizer options per set: HFT_SANITIZE / IDIOMS_SANITIZE = address|thread|none,
# DP_SANITIZE = ON|OFF.
cmake_minimum_required(VERSION 3.20)
project(cpp_practice CXX)
enable_testing()

add_subdirectory(hft-cpp)
add_subdirectory(dynamic-programming)
add_subdirectory(cpp-idioms)
