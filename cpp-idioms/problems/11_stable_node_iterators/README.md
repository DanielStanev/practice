# 11 · Stable iterators in node containers: an allocation-free LRU cache

| Group | Difficulty | Builds on |
|---|---|---|
| D. Iterator invalidation | ★★★☆☆ | 10 |

## The concept

Problem 10's table has a bright side: in node-based containers (`list`,
`map`, `set`, and the *elements* of `unordered_map`) an iterator stays valid
until **its own** element is erased. That means you can store iterators
inside other data structures as long-lived handles. Two more tools make
node containers much more useful than their reputation suggests:

**`std::list::splice`** moves nodes between positions (or between lists)
by relinking pointers. No allocation, no copy, no move of the element, and
**iterators to the moved nodes remain valid** (they now point into the
destination list).

```cpp
lst.splice(lst.begin(), lst, it);   // move *it to the front in O(1)
```

**Node handles (C++17)** let you take a node *out* of an associative
container and put it back (possibly with a new key) without freeing and
reallocating it:

```cpp
auto nh = map.extract(old_key);     // node now owned by `nh`, not freed
nh.key() = new_key;                 // keys are mutable while extracted
map.insert(std::move(nh));          // relinked, no allocation
```

### The classic: LRU cache

- `std::list<Entry>` holds entries in most-recently-used order.
- `std::unordered_map<Key, std::list<Entry>::iterator>` finds an entry's
  list node in O(1).
- `get` splices the node to the front. The map's stored iterator stays valid.
- When full, `put` of a new key evicts the back of the list.

Done naively, every eviction frees a list node and a map node and every
insert allocates two new ones. Allocation on the hot path is the thing HFT
interviewers will push you on. With splice and node handles, a full cache
can run forever **without allocating**: recycle the evicted list node (splice
it to the front and overwrite it) and the evicted map node (extract, rekey,
reinsert).

## Your task

Implement `idioms::LruCache<K, V>` in `lru_cache.hpp`:

- `explicit LruCache(std::size_t capacity)` (capacity ≥ 1).
- `V* get(const K& key)`: pointer to the value or `nullptr`; marks the key
  as most recently used.
- `void put(const K& key, V value)`: insert or overwrite; marks MRU; evicts
  the LRU entry if a new key arrives when full.
- `size()`, `capacity()`, `evictions()` (count so far).
- `std::vector<K> keys_mru_order() const` for the tests.

Rules checked by the tests (with `K = int`, `V = long`):

- `get` (hit or miss) never allocates;
- `put` to an existing key never allocates;
- once the cache is full, `put` of a new key never allocates
  (eviction recycles both nodes);
- a pointer returned by `get` stays valid across later `get`s and
  `put`s of *other* existing keys (node stability).

<details><summary>Hints</summary>

- Store the key in the list entry too, so eviction knows which map entry
  to extract.
- Call `map_.reserve(capacity)` in the constructor so the map never
  rehashes (a rehash allocates a new bucket array).
- Eviction: `auto last = std::prev(list_.end());`, extract
  `map_.extract(last->key)`, overwrite `*last`, splice it to the front,
  rekey the node handle, reinsert it with the same iterator value.
</details>
