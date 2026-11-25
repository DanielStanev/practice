# Optional CMake build (the per-problem run_tests.sh scripts need nothing but a compiler).
#
#   cmake -S . -B build -DIDIOMS_SANITIZE=address   # or thread, or none
#   cmake --build build -j
#   ctest --test-dir build --output-on-failure    # all problems
#   ctest --test-dir build -R 12_                 # one problem
#
cmake_minimum_required(VERSION 3.20)
project(cpp_idioms CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(IDIOMS_SANITIZE "address" CACHE STRING "Sanitizer for tests: address (ASan+UBSan), thread, or none")
set_property(CACHE IDIOMS_SANITIZE PROPERTY STRINGS address thread none)

find_package(Threads REQUIRED)
enable_testing()

set(IDIOMS_WARNINGS -Wall -Wextra -Wpedantic)
if(IDIOMS_SANITIZE STREQUAL "address")
  set(IDIOMS_SAN_FLAGS -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
elseif(IDIOMS_SANITIZE STREQUAL "thread")
  set(IDIOMS_SAN_FLAGS -fsanitize=thread)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND IDIOMS_SAN_FLAGS -Wno-tsan)
  endif()
else()
  set(IDIOMS_SAN_FLAGS "")
endif()

file(GLOB IDIOMS_PROBLEM_DIRS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/problems/*)
foreach(dir ${IDIOMS_PROBLEM_DIRS})
  if(NOT IS_DIRECTORY ${dir})
    continue()
  endif()
  get_filename_component(problem ${dir} NAME)

  file(GLOB test_src ${dir}/test_*.cpp)
  if(test_src)
    set(target test_${problem})
    add_executable(${target} ${test_src})
    target_include_directories(${target} PRIVATE ${dir} ${CMAKE_CURRENT_SOURCE_DIR}/common)
    target_compile_options(${target} PRIVATE -O2 -g ${IDIOMS_WARNINGS} ${IDIOMS_SAN_FLAGS})
    target_link_options(${target} PRIVATE ${IDIOMS_SAN_FLAGS})
    target_link_libraries(${target} PRIVATE Threads::Threads)
    add_test(NAME ${problem} COMMAND ${target})
    set_tests_properties(${problem} PROPERTIES TIMEOUT 300)
  endif()

  file(GLOB bench_src ${dir}/bench_*.cpp)
  if(bench_src)
    set(target bench_${problem})
    add_executable(${target} ${bench_src})
    target_include_directories(${target} PRIVATE ${dir} ${CMAKE_CURRENT_SOURCE_DIR}/common)
    target_compile_options(${target} PRIVATE -O3 -march=native -DNDEBUG ${IDIOMS_WARNINGS})
    target_link_libraries(${target} PRIVATE Threads::Threads)
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL ON)
  endif()
endforeach()
