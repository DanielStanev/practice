# Optional CMake build (the per-problem run_tests.sh scripts need nothing but a compiler).
#
#   cmake -S . -B build -DDP_SANITIZE=ON    # ASan + UBSan (default), or OFF
#   cmake --build build -j
#   ctest --test-dir build --output-on-failure   # all problems
#   ctest --test-dir build -R 09_                # one problem
cmake_minimum_required(VERSION 3.20)
project(dp_problems CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DP_SANITIZE "Build tests with AddressSanitizer and UndefinedBehaviorSanitizer" ON)

enable_testing()

set(DP_WARNINGS -Wall -Wextra -Wpedantic)
if(DP_SANITIZE)
  set(DP_SAN_FLAGS -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
else()
  set(DP_SAN_FLAGS "")
endif()

file(GLOB DP_PROBLEM_DIRS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/problems/*)
foreach(dir ${DP_PROBLEM_DIRS})
  if(NOT IS_DIRECTORY ${dir})
    continue()
  endif()
  get_filename_component(problem ${dir} NAME)
  file(GLOB test_src ${dir}/test_*.cpp)
  if(test_src)
    set(target test_${problem})
    add_executable(${target} ${test_src})
    target_include_directories(${target} PRIVATE ${dir} ${CMAKE_CURRENT_SOURCE_DIR}/common)
    target_compile_options(${target} PRIVATE -O2 -g ${DP_WARNINGS} ${DP_SAN_FLAGS})
    target_link_options(${target} PRIVATE ${DP_SAN_FLAGS})
    add_test(NAME ${problem} COMMAND ${target})
    set_tests_properties(${problem} PROPERTIES TIMEOUT 300)
  endif()
endforeach()
