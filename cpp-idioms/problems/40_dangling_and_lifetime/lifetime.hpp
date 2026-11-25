// Problem 40: object lifetime and dangling.
// Read README.md first. Implement everything marked TODO and fix the bugs.
#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace idioms {

// TODO: accept lvalue std::string and const char*, reject rvalue std::string.
inline std::string_view first_word(std::string_view s) {
    (void)s;
    throw std::logic_error("TODO: first_word");
}

class Instrument {
public:
    explicit Instrument(std::string s) : symbol_(std::move(s)) {}
    // TODO: ref-qualified overloads
    const std::string& symbol() const { throw std::logic_error("TODO: symbol"); }

private:
    std::string symbol_;
};

// The obvious version stores the view, which dangles once the caller's
// string is gone:
//
//   class Tag {
//   public:
//       explicit Tag(std::string_view name) : name_(name) {}
//       std::string_view name() const { return name_; }
//   private:
//       std::string_view name_;
//   };
//
// TODO: write a version that cannot dangle (keep the same interface).
class Tag {
public:
    explicit Tag(std::string_view name) { (void)name; }
    std::string_view name() const { throw std::logic_error("TODO: Tag"); }
};

inline std::string larger(const std::string& a, const std::string& b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: larger");
}

// The obvious version: every lambda captures `step` by reference, and `step`
// dies at the end of each loop iteration.
//
//   for (int i = 1; i <= 3; ++i) {
//       int step = i * 10;
//       out.push_back([&step](int x) { return x + step; });
//   }
//
// TODO: element i (0-based) adds (i + 1) * 10.
inline std::vector<std::function<int(int)>> make_adders() { throw std::logic_error("TODO: make_adders"); }

}  // namespace idioms
