// Problem 27: RAII for state and locks.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <atomic>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace idioms {

template <class T>
class SaveRestore {
public:
    explicit SaveRestore(T& var) : ref_(var) { throw std::logic_error("TODO: SaveRestore"); }
    // TODO: destructor, commit(), no copy/move
    void commit() noexcept {}

private:
    T& ref_;
};

class InFlight {
public:
    explicit InFlight(std::atomic<int>& counter) {
        (void)counter;
        throw std::logic_error("TODO: InFlight");
    }
    // TODO: destructor, move constructor, no copies
};

struct Account {
    std::mutex m;
    long balance = 0;
};

inline bool transfer(Account& from, Account& to, long amount) {
    (void)from;
    (void)to;
    (void)amount;
    throw std::logic_error("TODO: transfer");
}

inline long total(const std::vector<Account*>& accounts) {
    (void)accounts;
    throw std::logic_error("TODO: total");
}

}  // namespace idioms
