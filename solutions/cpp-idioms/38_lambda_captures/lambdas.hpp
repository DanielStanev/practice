// Reference solution for problem 38.
#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace idioms {

inline auto make_counter(int start) {
    return [n = start]() mutable { return n++; };
}

inline std::vector<std::function<int(int)>> make_multipliers(int n) {
    std::vector<std::function<int(int)>> out;
    for (int i = 0; i < n; ++i) out.push_back([i](int x) { return x * i; });
    return out;
}

template <class F, class... Fs>
auto compose(F f, Fs... fs) {
    if constexpr (sizeof...(Fs) == 0) {
        return f;
    } else {
        return [f, inner = compose(fs...)](auto&& x) { return f(inner(std::forward<decltype(x)>(x))); };
    }
}

template <class F>
auto memoize(F f) {
    return [f, cache = std::unordered_map<long, long>{}](long x) mutable {
        if (auto it = cache.find(x); it != cache.end()) return it->second;
        long v = f(x);
        cache.emplace(x, v);
        return v;
    };
}

inline auto make_owner(std::unique_ptr<int> p) {
    return [p = std::move(p)] { return *p; };
}

struct Order {
    int id;
    double price;
    std::function<std::string()> printer() const {
        return [*this] {
            std::ostringstream os;
            os << id << '@' << price;
            return os.str();
        };
    }
};

inline double tick_size(std::string_view venue, double price) {
    const double tick = [&] {
        if (venue == "XNAS") return price >= 1.0 ? 0.01 : 0.0001;
        if (venue == "XLON") return 0.5;
        return 0.01;
    }();
    return tick;
}

}  // namespace idioms
