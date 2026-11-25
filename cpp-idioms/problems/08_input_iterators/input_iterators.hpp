// Problem 08: single-pass input iterators and generators.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstddef>
#include <istream>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace idioms {

class Lines {
public:
    class iterator {
    public:
        // TODO: member types, postfix ++, comparison with std::default_sentinel_t
        const std::string& operator*() const { throw std::logic_error("TODO: Lines::iterator::operator*"); }
        iterator& operator++() { throw std::logic_error("TODO: Lines::iterator::operator++"); }

    private:
        friend class Lines;
        Lines* parent_ = nullptr;
    };

    explicit Lines(std::istream& in) : in_(&in) {}
    // TODO: not copyable

    iterator begin() { throw std::logic_error("TODO: Lines::begin"); }
    std::default_sentinel_t end() const { return {}; }

private:
    std::istream* in_;
    std::string line_;
};

template <class F>
class Generator {
public:
    using value_type = typename std::invoke_result_t<F&>::value_type;

    class iterator {
    public:
        // TODO: member types, operators
    private:
        friend class Generator;
        Generator* parent_ = nullptr;
    };

    explicit Generator(F f) : f_(std::move(f)) {}

    iterator begin() { throw std::logic_error("TODO: Generator::begin"); }
    std::default_sentinel_t end() const { return {}; }

private:
    F f_;
    std::optional<value_type> current_;
};

template <class F>
Generator<F> make_generator(F f) {
    return Generator<F>(std::move(f));
}

inline long count_errors(std::istream& in) {
    (void)in;
    throw std::logic_error("TODO: count_errors");
}

}  // namespace idioms
