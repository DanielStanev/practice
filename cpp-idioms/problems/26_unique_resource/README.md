# 26 · `UniqueResource`: RAII for handles that are not pointers

| Group | Difficulty | Builds on |
|---|---|---|
| I. RAII | ★★★☆☆ | 20, 25 |

## The concept

`std::unique_ptr<T, D>` is RAII for pointers. Trading systems are full of
resources that are **not** pointers: file descriptors and sockets (`int`),
`mmap` regions, `epoll` instances, shared memory segments, kernel-bypass NIC
handles, timer ids. Each needs exactly one release call, on every path.

You can bend `unique_ptr` to hold an `int` with a custom `pointer` type, but
the clean answer is a dedicated owner: `UniqueResource<R, D>` (proposed for
the standard as `std::experimental::unique_resource`). It owns a value of
type `R` and calls `D(r)` once when it is destroyed or reset.

```cpp
auto fd = make_unique_resource_checked(::open(path, O_RDONLY), -1, &::close);
if (!fd.valid()) throw std::system_error(errno, std::generic_category());
::read(fd.get(), buf, n);
// ::close runs automatically, exactly once, even if read throws later code
```

Design points that interviews dig into:

- **Ownership is unique**: no copying; moving transfers ownership and the
  source must no longer release anything.
- **"Is there anything to release?"** is separate from the value: an `int`
  of `0` is a valid fd. Track it with a `bool` (`execute_on_reset_`), and
  let `make_unique_resource_checked(r, invalid, d)` start disengaged when
  `r == invalid` (e.g. `open` returned `-1`).
- **`reset(new_r)`** releases the old resource first, then takes the new one.
- **`release()`** gives up ownership without calling the deleter (handing the
  fd to someone else).
- Move assignment must release the *current* resource before taking the
  other one, and self-move must be harmless.
- If the deleter is stateless (a function pointer or an empty lambda), the
  `UniqueResource<int, D>` should not be much bigger than the `int` and a
  `bool`. `[[no_unique_address]] D deleter_;` makes an empty deleter take
  no space.

## Your task

Implement in `unique_resource.hpp` (namespace `idioms`):

1. `UniqueResource<R, D>`: constructor `(R, D)` (engaged), default
   constructor (disengaged; needs default-constructible `R`, `D`), move
   constructor and move assignment, no copies, destructor, `reset()`,
   `reset(R)`, `release()`, `get()`, `get_deleter()`, `valid()`.
2. `make_unique_resource_checked(r, invalid, d)`.
3. `Fd`: an alias for a `UniqueResource` owning a POSIX fd closed with
   `::close`, and `open_pipe()` returning a `std::pair<Fd, Fd>` (read end,
   write end) created with `::pipe`, throwing `std::system_error` on failure.
4. An empty deleter takes no space: `sizeof(UniqueResource<char, EmptyDeleter>) == 2`
   (the handle plus the `bool`).

The tests count deleter calls and also check, with `fcntl(fd, F_GETFD)`,
that real fds are closed.

<details><summary>Hints</summary>

- A function used as a deleter: `UniqueResource<int, int (*)(int)>` with
  `&::close`. Since `close` is a C function, wrapping it in a lambda type
  (`struct CloseFd { void operator()(int fd) const noexcept { ::close(fd); } };`)
  gives you an empty deleter and the `[[no_unique_address]]` saving.
- Move assignment: `if (this != &o) { reset(); ...take o's state...; o.execute_on_reset_ = false; }`.
</details>
