# Optional CMake build (the per-problem run_tests.sh scripts need nothing but a compiler).
#
#   cmake -S . -B build -DHFT_SANITIZE=address   # or thread, or none
#   cmake --build build -j
#   ctest --test-dir build --output-on-failure    # all problems
#   ctest --test-dir build -R 12_                 # one problem
#
# Benchmarks (bench_*.cpp) are built with -O3 -march=native and no sanitizers:
#   cmake --build build --target bench_12_spsc_queue && ./build/bench_12_spsc_queue
cmake_minimum_required(VERSION 3.20)
project(hft_cpp_problems CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HFT_SANITIZE "address" CACHE STRING "Sanitizer for tests: address (ASan+UBSan), thread, or none")
set_property(CACHE HFT_SANITIZE PROPERTY STRINGS address thread none)

find_package(Threads REQUIRED)
enable_testing()

set(HFT_WARNINGS -Wall -Wextra -Wpedantic)
if(HFT_SANITIZE STREQUAL "address")
  set(HFT_SAN_FLAGS -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
elseif(HFT_SANITIZE STREQUAL "thread")
  set(HFT_SAN_FLAGS -fsanitize=thread)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND HFT_SAN_FLAGS -Wno-tsan)
  endif()
else()
  set(HFT_SAN_FLAGS "")
endif()

file(GLOB HFT_PROBLEM_DIRS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/problems/*)
foreach(dir ${HFT_PROBLEM_DIRS})
  if(NOT IS_DIRECTORY ${dir})
    continue()
  endif()
  get_filename_component(problem ${dir} NAME)

  file(GLOB test_src ${dir}/test_*.cpp)
  if(test_src)
    set(target test_${problem})
    add_executable(${target} ${test_src})
    target_include_directories(${target} PRIVATE ${dir} ${CMAKE_CURRENT_SOURCE_DIR}/common)
    target_compile_options(${target} PRIVATE -O2 -g ${HFT_WARNINGS} ${HFT_SAN_FLAGS})
    target_link_options(${target} PRIVATE ${HFT_SAN_FLAGS})
    target_link_libraries(${target} PRIVATE Threads::Threads)
    add_test(NAME ${problem} COMMAND ${target})
    set_tests_properties(${problem} PROPERTIES TIMEOUT 300)
  endif()

  file(GLOB bench_src ${dir}/bench_*.cpp)
  if(bench_src)
    set(target bench_${problem})
    add_executable(${target} ${bench_src})
    target_include_directories(${target} PRIVATE ${dir} ${CMAKE_CURRENT_SOURCE_DIR}/common)
    target_compile_options(${target} PRIVATE -O3 -march=native -DNDEBUG ${HFT_WARNINGS})
    target_link_libraries(${target} PRIVATE Threads::Threads)
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL ON)
  endif()
endforeach()
