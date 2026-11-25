// Reference solution for problem 27.
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
    explicit SaveRestore(T& var) : ref_(var), saved_(var) {}
    SaveRestore(const SaveRestore&) = delete;
    SaveRestore& operator=(const SaveRestore&) = delete;
    ~SaveRestore() {
        if (active_) ref_ = std::move(saved_);
    }
    void commit() noexcept { active_ = false; }

private:
    T& ref_;
    T saved_;
    bool active_ = true;
};

class InFlight {
public:
    explicit InFlight(std::atomic<int>& counter) : counter_(&counter) { counter_->fetch_add(1); }
    InFlight(const InFlight&) = delete;
    InFlight& operator=(const InFlight&) = delete;
    InFlight(InFlight&& o) noexcept : counter_(std::exchange(o.counter_, nullptr)) {}
    InFlight& operator=(InFlight&&) = delete;
    ~InFlight() {
        if (counter_) counter_->fetch_sub(1);
    }

private:
    std::atomic<int>* counter_;
};

struct Account {
    std::mutex m;
    long balance = 0;
};

inline bool transfer(Account& from, Account& to, long amount) {
    if (&from == &to) return false;
    std::scoped_lock lock(from.m, to.m);
    if (from.balance < amount) return false;
    from.balance -= amount;
    to.balance += amount;
    return true;
}

inline long total(const std::vector<Account*>& accounts) {
    long sum = 0;
    for (Account* a : accounts) {
        std::lock_guard lock(a->m);
        sum += a->balance;
    }
    return sum;
}

}  // namespace idioms
