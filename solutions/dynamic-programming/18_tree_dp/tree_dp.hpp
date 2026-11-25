// Problem 18: tree DP. Reference solution (iterative traversals).
#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dp {

using Edges = std::vector<std::pair<int, int>>;

namespace detail {
struct Rooted {
    std::vector<int> order;   // every node appears after its parent
    std::vector<int> parent;  // -1 for the root
};

inline Rooted root_tree(int n, const Edges& edges) {
    // Compressed adjacency (CSR): one allocation instead of n small vectors.
    std::vector<int> start(n + 1, 0), adj(2 * edges.size());
    for (auto [a, b] : edges) ++start[a + 1], ++start[b + 1];
    for (int i = 0; i < n; ++i) start[i + 1] += start[i];
    std::vector<int> fill(start.begin(), start.end() - 1);
    for (auto [a, b] : edges) adj[fill[a]++] = b, adj[fill[b]++] = a;

    Rooted r;
    r.parent.assign(n, -1);
    r.order.reserve(n);
    std::vector<char> seen(n, 0);
    std::vector<int> stack{0};
    seen[0] = 1;
    while (!stack.empty()) {
        int v = stack.back();
        stack.pop_back();
        r.order.push_back(v);
        for (int i = start[v]; i < start[v + 1]; ++i) {
            int c = adj[i];
            if (!seen[c]) {
                seen[c] = 1;
                r.parent[c] = v;
                stack.push_back(c);
            }
        }
    }
    return r;
}
}  // namespace detail

inline std::int64_t max_weight_independent_set(int n, const Edges& edges, const std::vector<std::int64_t>& weight) {
    auto t = detail::root_tree(n, edges);
    std::vector<std::int64_t> take(weight.begin(), weight.end()), skip(n, 0);
    for (auto it = t.order.rbegin(); it != t.order.rend(); ++it) {
        int v = *it, p = t.parent[v];
        if (p < 0) continue;
        take[p] += skip[v];
        skip[p] += std::max(take[v], skip[v]);
    }
    return std::max(take[0], skip[0]);
}

inline int tree_diameter(int n, const Edges& edges) {
    auto t = detail::root_tree(n, edges);
    std::vector<int> best1(n, 0), best2(n, 0);  // two largest (down[c] + 1) among children
    int answer = 0;
    for (auto it = t.order.rbegin(); it != t.order.rend(); ++it) {
        int v = *it;
        answer = std::max(answer, best1[v] + best2[v]);
        int p = t.parent[v];
        if (p < 0) continue;
        int len = best1[v] + 1;  // best1[v] is down[v]
        if (len > best1[p]) best2[p] = best1[p], best1[p] = len;
        else if (len > best2[p]) best2[p] = len;
    }
    return answer;
}

inline std::vector<std::int64_t> sum_of_distances(int n, const Edges& edges) {
    auto t = detail::root_tree(n, edges);
    std::vector<std::int64_t> size(n, 1), sub(n, 0);
    for (auto it = t.order.rbegin(); it != t.order.rend(); ++it) {
        int v = *it, p = t.parent[v];
        if (p < 0) continue;
        size[p] += size[v];
        sub[p] += sub[v] + size[v];
    }
    std::vector<std::int64_t> ans(n);
    ans[0] = sub[0];
    for (int v : t.order) {
        int p = t.parent[v];
        if (p >= 0) ans[v] = ans[p] - size[v] + (n - size[v]);
    }
    return ans;
}

}  // namespace dp
