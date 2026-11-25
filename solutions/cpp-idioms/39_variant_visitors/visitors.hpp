// Reference solution for problem 39.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace idioms {

template <class... Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};
template <class... Fs>
overloaded(Fs...) -> overloaded<Fs...>;

struct Add {
    std::uint64_t id;
    long qty;
    double price;
};
struct Cancel {
    std::uint64_t id;
};
struct Execute {
    std::uint64_t id;
    long qty;
};
using Msg = std::variant<Add, Cancel, Execute>;

inline std::string describe(const Msg& m) {
    std::ostringstream os;
    std::visit(overloaded{
                   [&](const Add& a) { os << "add " << a.qty << '@' << a.price; },
                   [&](const Cancel& c) { os << "cancel " << c.id; },
                   [&](const Execute& e) { os << "exec " << e.id << 'x' << e.qty; },
               },
               m);
    return os.str();
}

using Book = std::map<std::uint64_t, long>;

inline void apply_message(Book& book, const Msg& m) {
    std::visit(overloaded{
                   [&](const Add& a) { book[a.id] = a.qty; },
                   [&](const Cancel& c) { book.erase(c.id); },
                   [&](const Execute& e) {
                       auto it = book.find(e.id);
                       if (it == book.end()) return;
                       it->second -= e.qty;
                       if (it->second <= 0) book.erase(it);
                   },
               },
               m);
}

inline std::array<std::size_t, 3> count_by_type(const std::vector<Msg>& msgs) {
    std::array<std::size_t, 3> counts{};
    for (const Msg& m : msgs) {
        std::visit(
            [&](const auto& x) {
                using T = std::decay_t<decltype(x)>;
                if constexpr (std::is_same_v<T, Add>) ++counts[0];
                else if constexpr (std::is_same_v<T, Cancel>) ++counts[1];
                else ++counts[2];
            },
            m);
    }
    return counts;
}

struct Node {
    std::variant<long, std::vector<Node>> v;
};

inline long tree_sum(const Node& root) {
    auto go = [](auto self, const Node& n) -> long {
        return std::visit(overloaded{
                              [](long leaf) { return leaf; },
                              [&](const std::vector<Node>& kids) {
                                  long s = 0;
                                  for (const Node& k : kids) s += self(self, k);
                                  return s;
                              },
                          },
                          n.v);
    };
    return go(go, root);
}

}  // namespace idioms
