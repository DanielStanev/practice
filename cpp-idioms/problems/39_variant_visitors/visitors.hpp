// Problem 39: variant visitors and recursive lambdas.
// Read README.md first. Implement everything marked TODO.
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

// TODO: the overloaded idiom
template <class... Fs>
struct overloaded {};

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

// "add <qty>@<price>", "cancel <id>", "exec <id>x<qty>"
inline std::string describe(const Msg& m) {
    (void)m;
    throw std::logic_error("TODO: describe");
}

using Book = std::map<std::uint64_t, long>;  // id -> remaining qty

inline void apply_message(Book& book, const Msg& m) {
    (void)book;
    (void)m;
    throw std::logic_error("TODO: apply_message");
}

inline std::array<std::size_t, 3> count_by_type(const std::vector<Msg>& msgs) {
    (void)msgs;
    throw std::logic_error("TODO: count_by_type");
}

struct Node {
    std::variant<long, std::vector<Node>> v;
};

inline long tree_sum(const Node& root) {
    (void)root;
    throw std::logic_error("TODO: tree_sum");
}

}  // namespace idioms
