# 18 · Tree DP: subtrees as subproblems, and rerooting

| Difficulty | Builds on | New ideas |
|---|---|---|
| ★★★★☆ | 04, 05 | DP on a rooted tree (children before parent), multiple values per node, combining children, rerooting to get the answer for *every* root in O(n), iterative DFS to avoid stack overflow |

## Why this matters

Trees are everywhere: order hierarchies, org charts, dependency graphs,
network topologies, decision trees. In a tree, a node's **subtree** is a
natural subproblem: it only touches the rest of the tree through one edge.
Tree DP is also where recursion depth stops being theoretical: a path of
200 000 nodes will blow an 8 MB stack, and the tests check for it.

## The idea

Trees come as `n` nodes `0 … n-1` and `n-1` undirected edges. Pick node 0
as the root. Then every node has a parent and children, and its subtree's
answer only depends on its children's answers. So the order is
**children before parents** (post-order).

### Maximum-weight independent set (house robber on a tree)

Pick nodes with maximum total weight so that no two picked nodes are
adjacent. Problem 04 on a tree. For each node `v` keep **two** values:

```
take[v] = weight[v] + Σ_children skip[c]
skip[v] = Σ_children max(take[c], skip[c])
answer  = max(take[root], skip[root])
```

Two values per node because the parent's choice depends on whether the child
was taken: the "put the missing information into the state" lesson again.

### Diameter: combine the best two children

The diameter is the longest path (in edges) between any two nodes. For each
node let `down[v]` = longest path going **down** from `v`. The longest path
whose highest point is `v` uses the two largest `down[c] + 1` among its
children. So:

```
down[v] = max over children (down[c] + 1)        (0 for a leaf)
through[v] = sum of the two largest (down[c] + 1) (missing ones count 0)
answer = max over v of through[v]
```

That's "best ending here" (problem 05) on a tree: the answer is a max over
all nodes, not just the root's value.

### Rerooting: an answer for every node

`sum_of_distances[v]` = Σ over all `u` of dist(v, u), for every `v`.
Running a DFS from each node is O(n²): too slow for 200 000. Instead:

1. **Down pass** (post-order): `size[v]` = nodes in `v`'s subtree, and
   `sub[v]` = Σ distances from `v` to nodes in its subtree:
   `sub[v] = Σ_children (sub[c] + size[c])` (every node in `c`'s subtree is one edge further from `v`).
   Then `ans[root] = sub[root]`.
2. **Up pass** (pre-order, parents before children): moving the root from
   `p` to its child `c`, the `size[c]` nodes in `c`'s subtree get 1 closer
   and the other `n - size[c]` get 1 further:

```
ans[c] = ans[p] - size[c] + (n - size[c])
```

Two passes, O(n). Rerooting is a standard technique for "answer for every
root" questions; the pattern is always: compute subtree values, then push the
"rest of the tree" contribution downward.

### Don't recurse 200 000 deep

The default stack is usually 8 MB, and ASan makes frames bigger. A recursive
DFS on a path of 200 000 nodes **will crash** (the `deep_path` tests are
there to catch it). If the test run stops with `AddressSanitizer:
stack-overflow`, that's what happened. Do it iteratively:

1. Build adjacency lists. Do an explicit-stack DFS (or BFS) from the root,
   recording `parent[v]` and the visit `order`.
2. Process `order` **in reverse** for children-before-parents passes, and
   **forward** for parents-before-children passes.

This "compute an order once, then loop over it" pattern is also faster than
recursion and is how production code walks big trees.

## Task

Implement in `tree_dp.hpp` (`Edges` = `std::vector<std::pair<int, int>>`):

| Function | Returns |
|---|---|
| `max_weight_independent_set(n, edges, weight)` | max total weight of a set of pairwise non-adjacent nodes. Weights in `[0, 10^9]`. |
| `tree_diameter(n, edges)` | number of edges on the longest path |
| `sum_of_distances(n, edges)` | vector `ans` with `ans[v]` = Σ_u dist(v, u) |

`1 <= n <= 200000`; the edges form a tree.

## Requirements

- O(n) time for all three, and no recursion deeper than O(log n) or so.

<details>
<summary>Hint: building the order</summary>

```cpp
std::vector<int> order, parent(n, -1), stack{0};
std::vector<char> seen(n, 0);
seen[0] = 1;
while (!stack.empty()) {
    int v = stack.back(); stack.pop_back();
    order.push_back(v);
    for (int c : adj[v]) if (!seen[c]) { seen[c] = 1; parent[c] = v; stack.push_back(c); }
}
// every node appears after its parent in `order`
```
</details>

## Going further

- Weighted diameter, and returning the endpoints.
- Tree knapsack: pick exactly `k` nodes forming a connected subtree containing the root, maximizing weight (O(n·k) with the "merge children with size bounds" trick).
- Rerooting for max distance from every node (eccentricity): you need the top two child values per node.

## Reading

- *Competitive Programmer's Handbook* (Laaksonen), chapter 14 "Tree algorithms".
- Codeforces blog: "Rerooting technique" / "All-rerooting DP".
