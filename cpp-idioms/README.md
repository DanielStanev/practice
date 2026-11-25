# C++ Idioms and Iterators

42 C++20 problems in 15 groups that drill the parts of the language that
interviews for trading firms probe hardest and that real low-latency code
depends on: iterators (using them, writing them, not invalidating them),
the STL algorithms and ranges, value categories, moves and forwarding,
RAII, the rule of 0/3/5, templates, CRTP, compile-time programming,
lambdas, and the lifetime and undefined-behaviour traps.

Each problem's README **teaches the concept first** (what it is, why it
exists, how to use it, the pitfalls), then gives you a task. Every group
has two or three problems: the first introduces the idea, the next ones
make you apply it harder or combine it with something earlier.

This set complements the 20-problem HFT project ([`hft-cpp/`](../hft-cpp/)). Doing
groups A–J of this set first makes problems 01–10 there much easier.

## Quick start

Requirements: Linux (or WSL2), GCC 12+ or Clang 16+, Bash. CMake is optional.

```bash
cd problems/01_iterator_categories
cat README.md                   # the lesson and the task
$EDITOR iterator_categories.hpp # implement the TODOs
./run_tests.sh                  # build with -O2 + ASan + UBSan and run the tests
```

From this folder (the set root):

```bash
./run_all.sh                 # scoreboard for all 42 problems
./run_all.sh 19 20 21        # just those problems
./run_all.sh --compile-only  # check everything compiles
```

Per problem:

| Command | What it does |
|---|---|
| `./run_tests.sh` | ASan + UBSan (`-fno-sanitize-recover`: any UB aborts the run) |
| `./run_tests.sh asan move` | only tests whose name contains `move` |
| `./run_tests.sh release` | `-O3 -march=native`, no sanitizers |

`CXX=clang++ ./run_tests.sh` switches compilers. CMake users: see the top
of `CMakeLists.txt`.

All tests fail until you implement the header. That is expected: the stubs
compile and throw `std::logic_error("TODO: ...")`. Some stubs are
deliberately *incomplete* (an iterator missing its member types, a
`concept` defined as `false`); their tests report "not ready yet" instead
of breaking the build, and start running once the missing pieces exist.

## The problems

| # | Problem | Difficulty |
|---|---|---|
| **A** | **Iterator fundamentals** | |
| 01 | [Iterator categories and tag dispatch](problems/01_iterator_categories) | ★☆☆☆☆ |
| 02 | [Overloading generic algorithms on iterator concepts](problems/02_concept_overloading) | ★☆☆☆☆ |
| 03 | [The standard iterator adaptors](problems/03_iterator_adaptors) | ★☆☆☆☆ |
| **B** | **Writing iterators** | |
| 04 | [A forward iterator and its const twin](problems/04_forward_iterator) | ★★☆☆☆ |
| 05 | [A random access iterator for a ring buffer](problems/05_random_access_iterator) | ★★☆☆☆ |
| 06 | [A bidirectional iterator over a binary search tree](problems/06_bidirectional_iterator) | ★★★☆☆ |
| **C** | **The C++20 iterator model** | |
| 07 | [Sentinels: when `end()` is not an iterator](problems/07_sentinels) | ★★☆☆☆ |
| 08 | [Single-pass input iterators and generators](problems/08_input_iterators) | ★★☆☆☆ |
| 09 | [Output iterators: writing without a container](problems/09_output_iterators) | ★★☆☆☆ |
| **D** | **Iterator invalidation** | |
| 10 | [The rules and the idioms](problems/10_invalidation_rules) | ★★☆☆☆ |
| 11 | [Stable node iterators: an allocation-free LRU cache](problems/11_stable_node_iterators) | ★★★☆☆ |
| 12 | [Handles that never dangle: a generational slot map](problems/12_generational_handles) | ★★★☆☆ |
| **E** | **STL algorithms** | |
| 13 | [The algorithm toolkit: no raw loops](problems/13_algorithm_toolkit) | ★★☆☆☆ |
| 14 | [The binary search family and heterogeneous lookup](problems/14_binary_search) | ★★☆☆☆ |
| 15 | [Writing generic algorithms like the standard library](problems/15_write_your_own_algorithms) | ★★★☆☆ |
| **F** | **Ranges** | |
| 16 | [Views: lazy pipelines](problems/16_views_pipelines) | ★★☆☆☆ |
| 17 | [Writing your own view and range adaptor](problems/17_custom_views) | ★★★★☆ |
| 18 | [Projections, borrowed ranges and `dangling`](problems/18_projections_and_dangling) | ★★★☆☆ |
| **G** | **Value categories and moves** | |
| 19 | [lvalues, prvalues, xvalues](problems/19_value_categories) | ★★☆☆☆ |
| 20 | [Move semantics: stealing, sinks and ref-qualifiers](problems/20_move_semantics) | ★★☆☆☆ |
| 21 | [`noexcept`, `move_if_noexcept`, implicit moves on return](problems/21_noexcept_and_relocation) | ★★★☆☆ |
| **H** | **Perfect forwarding** | |
| 22 | [Perfect forwarding and `decltype(auto)`](problems/22_perfect_forwarding) | ★★☆☆☆ |
| 23 | [`std::invoke`, member pointers and `bind_front`](problems/23_invoke_and_bind) | ★★★☆☆ |
| 24 | [Forwarding pitfalls](problems/24_forwarding_pitfalls) | ★★★☆☆ |
| **I** | **RAII** | |
| 25 | [Scope guards: exit, fail, success](problems/25_scope_guards) | ★★☆☆☆ |
| 26 | [`UniqueResource` for handles that are not pointers](problems/26_unique_resource) | ★★★☆☆ |
| 27 | [RAII for state and locks](problems/27_raii_locks_and_state) | ★★★☆☆ |
| **J** | **Rule of 0/3/5** | |
| 28 | [The rule of five with two resources](problems/28_rule_of_five_exception_safety) | ★★★☆☆ |
| 29 | [The rule of zero and polymorphic value types](problems/29_rule_of_zero) | ★★★☆☆ |
| 30 | [Which special members does the compiler give you?](problems/30_special_member_rules) | ★★★☆☆ |
| **K** | **Templates, SFINAE, concepts** | |
| 31 | [SFINAE and the detection idiom](problems/31_sfinae_detection) | ★★★☆☆ |
| 32 | [Concepts and subsumption](problems/32_concepts) | ★★★☆☆ |
| 33 | [Variadic templates, folds, index sequences](problems/33_variadic_templates) | ★★★☆☆ |
| **L** | **CRTP** | |
| 34 | [CRTP mixins](problems/34_crtp_mixins) | ★★★☆☆ |
| 35 | [CRTP static dispatch: a feed handler with no virtual calls](problems/35_crtp_static_dispatch) | ★★★★☆ |
| **M** | **Compile-time programming** | |
| 36 | [`constexpr` and `consteval` tables](problems/36_constexpr_tables) | ★★★☆☆ |
| 37 | [Strings as template parameters, constexpr containers](problems/37_fixed_string_nttp) | ★★★★☆ |
| **N** | **Lambdas** | |
| 38 | [What a capture really is](problems/38_lambda_captures) | ★★☆☆☆ |
| 39 | [`std::variant`, `overloaded` and recursive lambdas](problems/39_variant_visitors) | ★★★☆☆ |
| **O** | **Lifetime and undefined behaviour** | |
| 40 | [Dangling references and how to prevent them](problems/40_dangling_and_lifetime) | ★★★☆☆ |
| 41 | [Bytes, type punning, alignment, strict aliasing](problems/41_type_punning_and_alignment) | ★★★☆☆ |
| 42 | [Integer arithmetic without UB](problems/42_integer_ub) | ★★★☆☆ |

The order is a suggestion. Groups A–F (iterators, algorithms, ranges) and
G–J (values, moves, resources) are two mostly independent tracks you can
interleave; K–O lean on both.

## How to work through it

1. Read the README's concept section *before* looking at the header. If
   any of it is new, write a 10-line experiment in Compiler Explorer
   (godbolt.org) first.
2. Implement, run the tests, and when one fails, read the test: the tests
   are part of the lesson (many contain the edge case the README warns
   about).
3. Read the hints only after you are stuck for a while.
4. When everything passes, re-read the concept section and check you can
   explain every pitfall in it out loud. That is what the interview is.

Reference solutions live on the `solutions` branch, not here, so you are not
tempted. See the top-level README for how to view them.

## Layout

```
common/
  testing.hpp        minimal test harness (TEST, CHECK, CHECK_EQ, CHECK_TRAIT, ...)
  alloc_counter.hpp  counts operator new calls, for "must not allocate" tests
  run_problem.sh     build/run logic shared by every run_tests.sh
problems/NN_name/
  README.md          the lesson and the task
  <name>.hpp         the file you edit
  test_<name>.cpp    the tests
  run_tests.sh       build and run
run_all.sh           scoreboard
CMakeLists.txt       optional CMake/CTest build
```

## Further reading

- Nicolai Josuttis, *C++20 – The Complete Guide* (ranges, concepts, NTTPs).
- Scott Meyers, *Effective Modern C++* (items 23–30 are problems 19–24).
- Arthur O'Dwyer's blog (quuxplusone.github.io): iterators, value
  categories, "the rule of zero" and many of the pitfalls here.
- Howard Hinnant's special-members table (problem 30), from his talk
  "Everything You Ever Wanted To Know About Move Semantics".
- cppreference.com, especially the *Iterator library*, *Ranges library* and
  *Value categories* pages.
