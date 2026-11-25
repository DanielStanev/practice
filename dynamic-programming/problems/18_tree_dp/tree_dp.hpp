// Problem 18: tree DP.
// Read README.md first. Implement everything marked TODO.
// Beware: trees can be paths of 200000 nodes. Recursive DFS will overflow the stack.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dp {

using Edges = std::vector<std::pair<int, int>>;

// Max total weight of a set of nodes with no two adjacent.
inline std::int64_t max_weight_independent_set(int n, const Edges& edges, const std::vector<std::int64_t>& weight) {
    (void)n;
    (void)edges;
    (void)weight;
    throw std::logic_error("TODO: max_weight_independent_set");
}

// Number of edges on the longest simple path.
inline int tree_diameter(int n, const Edges& edges) {
    (void)n;
    (void)edges;
    throw std::logic_error("TODO: tree_diameter");
}

// ans[v] = sum over all nodes u of the number of edges between v and u.
inline std::vector<std::int64_t> sum_of_distances(int n, const Edges& edges) {
    (void)n;
    (void)edges;
    throw std::logic_error("TODO: sum_of_distances");
}

}  // namespace dp
