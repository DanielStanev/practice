// Problem 17: writing your own views and adaptors.
// Read README.md first. The views below have no begin()/end() yet.
#pragma once

#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace idioms {

template <std::ranges::view V>
class StrideView : public std::ranges::view_interface<StrideView<V>> {
public:
    StrideView() requires std::default_initializable<V> = default;
    StrideView(V base, std::ranges::range_difference_t<V> n) : base_(std::move(base)), n_(n) {}

    // TODO: iterator class, begin(), end()

private:
    V base_ = V();
    std::ranges::range_difference_t<V> n_ = 1;
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
class PairwiseView : public std::ranges::view_interface<PairwiseView<V>> {
public:
    PairwiseView() requires std::default_initializable<V> = default;
    explicit PairwiseView(V base) : base_(std::move(base)) {}

    // TODO: iterator class, begin(), end()

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
    (void)prices;
    throw std::logic_error("TODO: spreads");
}

}  // namespace idioms
