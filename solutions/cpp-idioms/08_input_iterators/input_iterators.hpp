// Reference solution for problem 08.
#pragma once

#include <algorithm>
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
        using value_type = std::string;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::input_iterator_tag;

        iterator() = default;
        const std::string& operator*() const { return parent_->line_; }
        iterator& operator++() {
            parent_->advance();
            return *this;
        }
        void operator++(int) { ++*this; }
        bool operator==(std::default_sentinel_t) const { return parent_->done_; }

    private:
        friend class Lines;
        explicit iterator(Lines* p) : parent_(p) {}
        Lines* parent_ = nullptr;
    };

    explicit Lines(std::istream& in) : in_(&in) {}
    Lines(const Lines&) = delete;
    Lines& operator=(const Lines&) = delete;

    iterator begin() {
        advance();
        return iterator(this);
    }
    std::default_sentinel_t end() const { return {}; }

private:
    void advance() { done_ = !std::getline(*in_, line_); }
    std::istream* in_;
    std::string line_;
    bool done_ = false;
};

template <class F>
class Generator {
public:
    using value_type = typename std::invoke_result_t<F&>::value_type;

    class iterator {
    public:
        using value_type = Generator::value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::input_iterator_tag;

        iterator() = default;
        const value_type& operator*() const { return *parent_->current_; }
        iterator& operator++() {
            parent_->current_ = parent_->f_();
            return *this;
        }
        void operator++(int) { ++*this; }
        bool operator==(std::default_sentinel_t) const { return !parent_->current_.has_value(); }

    private:
        friend class Generator;
        explicit iterator(Generator* p) : parent_(p) {}
        Generator* parent_ = nullptr;
    };

    explicit Generator(F f) : f_(std::move(f)) {}

    iterator begin() {
        current_ = f_();
        return iterator(this);
    }
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
    Lines lines(in);
    return static_cast<long>(
        std::ranges::count_if(lines, [](const std::string& l) { return l.starts_with("ERROR"); }));
}

}  // namespace idioms
