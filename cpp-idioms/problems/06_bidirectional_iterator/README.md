# 06 · A bidirectional iterator over a binary search tree

| Group | Difficulty | Builds on |
|---|---|---|
| B. Writing iterators | ★★★☆☆ | 04, 05 |

## The concept

`std::map` and `std::set` are red-black trees, and their iterators walk the
tree **in order** one node at a time. Understanding how is useful twice
over: it explains their complexity (`++it` is amortised O(1), worst case
O(log n)) and it is a favourite interview question.

### In-order successor with parent pointers

```
            50
          /    \
        30      70
       /  \       \
     20    40      80
```

- If the node has a right child: the successor is the **leftmost** node of
  the right subtree (`30 -> 40`, `50 -> 70`).
- Otherwise: walk up while you are a right child; the parent you reach from
  the left is the successor (`40 -> 50`). If you run off the root, you were
  at the maximum and the result is `end()`.

The predecessor (`--it`) is the mirror image.

### Making `--end()` work

Every standard bidirectional container allows `std::prev(c.end())` to reach
the last element, which is what makes `rbegin()` possible
(`reverse_iterator(end())` dereferences `*std::prev(end())`). So the end
iterator must know how to find the maximum. Two common designs:

1. A **header/sentinel node** whose left child is the root (libstdc++ does a
   variation of this): end() points at the header, and the general
   predecessor rule then works with no special case.
2. The iterator stores the tree pointer too, and `end()` is `{tree, nullptr}`;
   `--` on a null node jumps to the rightmost node.

Option 2 is simpler to get right; option 1 is what you would do for speed.

### Set iterators are always const

Changing a key through an iterator would break the tree's ordering, so both
`std::set::iterator` and `const_iterator` yield `const T&`. You only need one
iterator type here.

## Your task

Implement `idioms::OrderedSet<T>` in `ordered_set.hpp`: an unbalanced
BST of unique keys owning its nodes with `std::unique_ptr` (no manual
`delete`), with parent pointers.

- `std::pair<iterator, bool> insert(T)`: like `std::set::insert`.
- `iterator find(const T&) const`, `iterator lower_bound(const T&) const`.
- `size()`, `begin()`, `end()`, `rbegin()`, `rend()`.
- `iterator` satisfies `std::bidirectional_iterator`, yields `const T&`,
  and `--end()` reaches the largest key.
- Iterators stay valid when other keys are inserted (node-based container).

Not required: erase and balancing. (Do note what a sorted input does to an
unbalanced tree's depth. The destructor of a 20 000-deep `unique_ptr` chain
recurses 20 000 times; the test does that on a thread with a 256 KiB stack, so make the
destructor iterative or the stack overflows.)

<details><summary>Hints</summary>

- `Node { T value; Node* parent; std::unique_ptr<Node> left, right; }`.
- Iterative destructor: repeatedly detach children into a local
  `std::vector<std::unique_ptr<Node>>` stack before the node dies.
- `begin()` is the leftmost node. Walk with `.get()` pointers; only
  `insert` touches ownership.
</details>
