// Minimal single-header test harness used by every problem.
//
// You should not need to edit this file. Tests look like:
//
//   TEST(push_then_pop) {
//       Queue q;
//       q.push(1);
//       CHECK_EQ(q.pop(), 1);
//   }
//
//   int main(int argc, char** argv) { return testing::run_all(argc, argv); }
//
// Every CHECK is fatal for the current test (like gtest's ASSERT_*). Any
// exception escaping a test is reported as a failure, which is how the
// "not implemented" stubs show up before you start.
//
// Command line:  ./test_binary [substring]   runs only tests whose name contains substring
#pragma once

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace testing {

// True when compiled with ThreadSanitizer. TSan slows atomics-heavy code by
// 10-100x, so stress tests use smaller iteration counts under it.
#if defined(__SANITIZE_THREAD__)
inline constexpr bool kThreadSanitizer = true;
#elif defined(__has_feature)
#if __has_feature(thread_sanitizer)
inline constexpr bool kThreadSanitizer = true;
#else
inline constexpr bool kThreadSanitizer = false;
#endif
#else
inline constexpr bool kThreadSanitizer = false;
#endif

// Iteration count for stress tests: `normal`, or normal/10 under TSan.
constexpr std::uint64_t stress(std::uint64_t normal) {
    return kThreadSanitizer ? (normal / 10 > 0 ? normal / 10 : 1) : normal;
}

struct TestCase {
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

struct AssertionFailure {
    std::string message;
};

template <typename T>
concept Streamable = requires(std::ostream& os, const T& v) { os << v; };

template <typename T>
std::string describe(const T& v) {
    if constexpr (std::is_same_v<T, bool>) {
        return v ? "true" : "false";
    } else if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> ||
                         std::is_same_v<T, unsigned char>) {
        std::ostringstream os;
        os << static_cast<int>(v);
        return os.str();
    } else if constexpr (std::is_enum_v<T>) {
        std::ostringstream os;
        os << static_cast<long long>(static_cast<std::underlying_type_t<T>>(v));
        return os.str();
    } else if constexpr (Streamable<T>) {
        std::ostringstream os;
        os << v;
        return os.str();
    } else {
        return "<unprintable>";
    }
}

[[noreturn]] inline void fail(const char* file, int line, const std::string& what) {
    std::ostringstream os;
    os << file << ":" << line << ": " << what;
    throw AssertionFailure{os.str()};
}

template <typename A, typename B>
void check_eq(const A& a, const B& b, const char* ea, const char* eb, const char* file, int line) {
    if (!(a == b)) {
        fail(file, line,
             std::string("CHECK_EQ(") + ea + ", " + eb + ")\n      left:  " + describe(a) +
                 "\n      right: " + describe(b));
    }
}

inline int run_all(int argc, char** argv) {
    const char* filter = argc > 1 ? argv[1] : nullptr;
    int passed = 0, failed = 0;
    std::vector<std::string> failures;
    for (const auto& t : registry()) {
        if (filter && !std::strstr(t.name, filter)) continue;
        std::printf("[ RUN      ] %s\n", t.name);
        std::fflush(stdout);
        auto start = std::chrono::steady_clock::now();
        std::string error;
        try {
            t.fn();
        } catch (const AssertionFailure& f) {
            error = f.message;
        } catch (const std::exception& e) {
            error = std::string("uncaught exception: ") + e.what();
        } catch (...) {
            error = "uncaught non-std exception";
        }
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - start)
                      .count();
        if (error.empty()) {
            ++passed;
            std::printf("[       OK ] %s (%lld ms)\n", t.name, static_cast<long long>(ms));
        } else {
            ++failed;
            failures.emplace_back(t.name);
            std::printf("    %s\n[  FAILED  ] %s (%lld ms)\n", error.c_str(), t.name,
                        static_cast<long long>(ms));
        }
        std::fflush(stdout);
    }
    std::printf("\n%d passed, %d failed\n", passed, failed);
    for (const auto& f : failures) std::printf("  FAILED: %s\n", f.c_str());
    std::fflush(stdout);
    // When tests already failed, exit without running atexit handlers so the
    // leak checker does not bury the failures under leaks from aborted tests.
    // Once everything passes, leaks are reported (and fail the run) as usual.
    if (failed != 0) std::_Exit(1);
    return 0;
}

}  // namespace testing

#define TESTING_CAT2(a, b) a##b
#define TESTING_CAT(a, b) TESTING_CAT2(a, b)

#define TEST(name)                                                              \
    static void TESTING_CAT(test_fn_, name)();                                  \
    static ::testing::Registrar TESTING_CAT(test_reg_, name)(#name,             \
                                                             TESTING_CAT(test_fn_, name)); \
    static void TESTING_CAT(test_fn_, name)()

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) ::testing::fail(__FILE__, __LINE__, "CHECK(" #cond ")");  \
    } while (0)

#define CHECK_FALSE(cond) CHECK(!(cond))

#define CHECK_EQ(a, b) ::testing::check_eq((a), (b), #a, #b, __FILE__, __LINE__)

#define CHECK_NEAR(a, b, eps)                                                          \
    do {                                                                               \
        auto testing_a_ = (a);                                                         \
        auto testing_b_ = (b);                                                         \
        auto testing_d_ = testing_a_ > testing_b_ ? testing_a_ - testing_b_            \
                                                  : testing_b_ - testing_a_;           \
        if (testing_d_ > (eps))                                                        \
            ::testing::fail(__FILE__, __LINE__,                                        \
                            "CHECK_NEAR(" #a ", " #b ", " #eps ")\n      left:  " +    \
                                ::testing::describe(testing_a_) +                      \
                                "\n      right: " + ::testing::describe(testing_b_));  \
    } while (0)

#define CHECK_THROWS_AS(expr, ExType)                                               \
    do {                                                                            \
        bool testing_caught_ = false;                                               \
        try {                                                                       \
            (void)(expr);                                                           \
        } catch (const ExType&) {                                                   \
            testing_caught_ = true;                                                 \
        }                                                                           \
        if (!testing_caught_)                                                       \
            ::testing::fail(__FILE__, __LINE__,                                     \
                            "CHECK_THROWS_AS(" #expr ", " #ExType "): nothing thrown"); \
    } while (0)

// Compile-time requirement that is reported as a normal test failure rather
// than breaking the build, so unfinished problems still compile.
#define CHECK_TRAIT(...) CHECK((__VA_ARGS__))
