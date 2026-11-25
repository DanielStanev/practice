// Reference solution for problem 17.
#pragma once

#include <cmath>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace idioms {

template <std::ranges::view V>
class StrideView : public std::ranges::view_interface<StrideView<V>> {
    using BaseIt = std::ranges::iterator_t<V>;
    using BaseEnd = std::ranges::sentinel_t<V>;
    using Diff = std::ranges::range_difference_t<V>;

public:
    class iterator {
    public:
        using value_type = std::ranges::range_value_t<V>;
        using difference_type = Diff;
        using iterator_concept =
            std::conditional_t<std::ranges::forward_range<V>, std::forward_iterator_tag, std::input_iterator_tag>;
        using iterator_category = std::input_iterator_tag;

        iterator() = default;
        iterator(BaseIt cur, BaseEnd end, Diff n) : current_(std::move(cur)), end_(std::move(end)), n_(n) {}

        decltype(auto) operator*() const { return *current_; }
        iterator& operator++() {
            std::ranges::advance(current_, n_, end_);
            return *this;
        }
        iterator operator++(int) requires std::ranges::forward_range<V> {
            iterator t = *this;
            ++*this;
            return t;
        }
        void operator++(int) requires(!std::ranges::forward_range<V>) { ++*this; }
        bool operator==(const iterator& o) const requires std::equality_comparable<BaseIt> {
            return current_ == o.current_;
        }
        bool operator==(std::default_sentinel_t) const { return current_ == end_; }

    private:
        BaseIt current_{};
        BaseEnd end_{};
        Diff n_ = 1;
    };

    StrideView() requires std::default_initializable<V> = default;
    StrideView(V base, Diff n) : base_(std::move(base)), n_(n) {}

    iterator begin() { return iterator(std::ranges::begin(base_), std::ranges::end(base_), n_); }
    std::default_sentinel_t end() const { return {}; }

private:
    V base_ = V();
    Diff n_ = 1;
};

template <class R>
StrideView(R&&, std::ranges::range_difference_t<R>) -> StrideView<std::views::all_t<R>>;

struct StrideAdaptor {
    std::ptrdiff_t n;
    template <std::ranges::viewable_range R>
    friend auto operator|(R&& r, StrideAdaptor a) {
        return StrideView(std::views::all(std::forward<R>(r)), a.n);
    }
};
inline StrideAdaptor stride(std::ptrdiff_t n) { return {n}; }

template <std::ranges::view V>
    requires std::ranges::forward_range<V>
class PairwiseView : public std::ranges::view_interface<PairwiseView<V>> {
    using BaseIt = std::ranges::iterator_t<V>;
    using BaseEnd = std::ranges::sentinel_t<V>;
    using T = std::ranges::range_value_t<V>;

public:
    class iterator {
    public:
        using value_type = std::pair<T, T>;
        using difference_type = std::ranges::range_difference_t<V>;
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::input_iterator_tag;

        iterator() = default;
        iterator(BaseIt cur, BaseIt next, BaseEnd end) : current_(cur), next_(next), end_(end) {}

        value_type operator*() const { return {*current_, *next_}; }
        iterator& operator++() {
            current_ = next_;
            ++next_;
            return *this;
        }
        iterator operator++(int) {
            iterator t = *this;
            ++*this;
            return t;
        }
        bool operator==(const iterator& o) const { return next_ == o.next_; }
        bool operator==(std::default_sentinel_t) const { return next_ == end_; }

    private:
        BaseIt current_{};
        BaseIt next_{};
        BaseEnd end_{};
    };

    PairwiseView() requires std::default_initializable<V> = default;
    explicit PairwiseView(V base) : base_(std::move(base)) {}

    iterator begin() {
        BaseIt first = std::ranges::begin(base_);
        BaseIt next = first;
        if (first != std::ranges::end(base_)) ++next;
        return iterator(first, next, std::ranges::end(base_));
    }
    std::default_sentinel_t end() const { return {}; }

private:
    V base_ = V();
};

template <class R>
PairwiseView(R&&) -> PairwiseView<std::views::all_t<R>>;

struct PairwiseAdaptor {
    template <std::ranges::viewable_range R>
    friend auto operator|(R&& r, PairwiseAdaptor) {
        return PairwiseView(std::views::all(std::forward<R>(r)));
    }
};
inline constexpr PairwiseAdaptor pairwise{};

inline std::vector<double> spreads(const std::vector<double>& prices) {
    std::vector<double> out;
    for (auto [a, b] : prices | pairwise) out.push_back(std::abs(b - a));
    return out;
}

}  // namespace idioms
