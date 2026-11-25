// Problem 12: edit distance.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace dp {

enum class Op { Keep, Replace, Insert, Delete };

struct Edit {
    Op op;
    char ch;  // the character written for Replace and Insert; ignored for Keep and Delete
};

// Minimum number of single-character inserts, deletes and replaces turning a into b.
inline int edit_distance(std::string_view a, std::string_view b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: edit_distance");
}

// A minimal alignment script (see README) that turns a into b.
inline std::vector<Edit> edit_script(std::string_view a, std::string_view b) {
    (void)a;
    (void)b;
    throw std::logic_error("TODO: edit_script");
}

// Minimum total cost with the given per-operation costs.
inline std::int64_t edit_distance_weighted(std::string_view a, std::string_view b, std::int64_t insert_cost,
                                           std::int64_t delete_cost, std::int64_t replace_cost) {
    (void)a;
    (void)b;
    (void)insert_cost;
    (void)delete_cost;
    (void)replace_cost;
    throw std::logic_error("TODO: edit_distance_weighted");
}

}  // namespace dp
