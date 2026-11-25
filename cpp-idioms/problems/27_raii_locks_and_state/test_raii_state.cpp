#include "raii_state.hpp"

#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

TEST(save_restore_restores) {
    std::string level = "info";
    {
        SaveRestore g(level);
        level = "debug";
        CHECK_EQ(level, std::string("debug"));
    }
    CHECK_EQ(level, std::string("info"));
    try {
        SaveRestore g(level);
        level = "trace";
        throw std::runtime_error("x");
    } catch (const std::runtime_error&) {
    }
    CHECK_EQ(level, std::string("info"));
}

TEST(save_restore_commit) {
    int x = 1;
    {
        SaveRestore g(x);
        x = 2;
        g.commit();
    }
    CHECK_EQ(x, 2);
    CHECK_TRAIT(!std::is_copy_constructible_v<SaveRestore<int>>);
    CHECK_TRAIT(!std::is_move_constructible_v<SaveRestore<int>>);
}

TEST(in_flight_counts) {
    std::atomic<int> n{0};
    {
        InFlight a(n);
        CHECK_EQ(n.load(), 1);
        {
            InFlight b(n);
            CHECK_EQ(n.load(), 2);
        }
        CHECK_EQ(n.load(), 1);
        InFlight c(std::move(a));
        CHECK_EQ(n.load(), 1);
    }
    CHECK_EQ(n.load(), 0);
    CHECK_TRAIT(!std::is_copy_constructible_v<InFlight>);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<InFlight>);
    std::vector<InFlight> v;
    for (int i = 0; i < 10; ++i) v.emplace_back(n);  // growth moves guards around
    CHECK_EQ(n.load(), 10);
    v.clear();
    CHECK_EQ(n.load(), 0);
}

TEST(transfer_rules) {
    Account a, b;
    a.balance = 100;
    CHECK(transfer(a, b, 60));
    CHECK_FALSE(transfer(a, b, 60));
    CHECK_FALSE(transfer(a, a, 10));
    CHECK_EQ(a.balance, 40L);
    CHECK_EQ(b.balance, 60L);
}

TEST(transfer_no_deadlock_under_contention) {
    Account a, b;
    a.balance = 1000;
    b.balance = 1000;
    const int iters = 200000;
    std::atomic<bool> failed{false};
    // Exceptions must not escape a thread (std::terminate), so catch and record.
    auto run = [&](Account& from, Account& to) {
        try {
            for (int i = 0; i < iters; ++i) transfer(from, to, 1);
        } catch (...) {
            failed = true;
        }
    };
    std::thread t1(run, std::ref(a), std::ref(b));
    std::thread t2(run, std::ref(b), std::ref(a));
    std::atomic<bool> stop{false};
    std::thread auditor([&] {
        try {
            while (!stop.load()) total({&a, &b});
        } catch (...) {
            failed = true;
        }
    });
    t1.join();
    t2.join();
    stop = true;
    auditor.join();
    CHECK_FALSE(failed.load());
    CHECK_EQ(a.balance + b.balance, 2000L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
