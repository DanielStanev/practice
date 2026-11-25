// Problem 13: Seqlock (single writer, many readers, readers never block the writer)
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
//
// Threading contract: one writer thread calls store(); any number of
// reader threads call load() / try_load() / version().
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace hft {

template <typename T>
class Seqlock {
    static_assert(std::is_trivially_copyable_v<T>, "Seqlock<T> requires a trivially copyable T");

public:
    Seqlock() : Seqlock(T{}) {}
    explicit Seqlock(const T& initial) {
        (void)initial;
        // TODO
    }

    // Writer. Never blocks, never waits for readers.
    void store(const T& value) {
        (void)value;
        throw std::logic_error("TODO: store");
    }

    // Reader. One attempt: returns false if a write was in progress or
    // happened during the copy. On true, out holds a consistent snapshot.
    bool try_load(T& out) const {
        (void)out;
        throw std::logic_error("TODO: try_load");
    }

    // Reader. Retries until it gets a consistent snapshot.
    T load() const { throw std::logic_error("TODO: load"); }

    // Number of completed store() calls.
    std::uint64_t version() const { throw std::logic_error("TODO: version"); }

private:
    // TODO: a sequence counter and storage for T that can be read
    // concurrently with writes WITHOUT a data race (ThreadSanitizer checks).
};

}  // namespace hft
