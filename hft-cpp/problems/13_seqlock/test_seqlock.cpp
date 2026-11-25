#include "seqlock.hpp"

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

#include "testing.hpp"

using hft::Seqlock;

namespace {

// A top-of-book snapshot whose fields are all derived from `seq`, so a
// reader can detect a torn (half old, half new) copy.
struct TopOfBook {
    std::uint64_t seq;
    std::int64_t bid_px;
    std::int64_t ask_px;
    std::uint32_t bid_qty;
    std::uint32_t ask_qty;
    std::uint64_t check[4];

    static TopOfBook make(std::uint64_t s) {
        TopOfBook t{};
        t.seq = s;
        t.bid_px = 10000 + std::int64_t(s % 100);
        t.ask_px = t.bid_px + 1;
        t.bid_qty = std::uint32_t(s * 3);
        t.ask_qty = std::uint32_t(s * 7);
        for (int i = 0; i < 4; ++i) t.check[i] = s ^ (0x9e3779b97f4a7c15ull * (i + 1));
        return t;
    }
    bool consistent() const {
        TopOfBook e = make(seq);
        if (bid_px != e.bid_px || ask_px != e.ask_px || bid_qty != e.bid_qty || ask_qty != e.ask_qty)
            return false;
        for (int i = 0; i < 4; ++i)
            if (check[i] != e.check[i]) return false;
        return true;
    }
};

// Odd size, not a multiple of 8.
struct Small {
    char c[13];
};

}  // namespace

TEST(store_then_load) {
    Seqlock<TopOfBook> s;
    TopOfBook initial = s.load();
    CHECK_EQ(initial.seq, 0u);
    s.store(TopOfBook::make(5));
    TopOfBook t = s.load();
    CHECK_EQ(t.seq, 5u);
    CHECK(t.consistent());
}

TEST(initial_value_constructor) {
    Seqlock<TopOfBook> s(TopOfBook::make(42));
    CHECK_EQ(s.load().seq, 42u);
    CHECK_EQ(s.version(), 0u);
}

TEST(version_counts_stores) {
    Seqlock<std::uint64_t> s;
    CHECK_EQ(s.version(), 0u);
    for (std::uint64_t i = 1; i <= 10; ++i) {
        s.store(i);
        CHECK_EQ(s.version(), i);
    }
}

TEST(try_load_succeeds_when_quiet) {
    Seqlock<Small> s;
    Small v{};
    for (int i = 0; i < 13; ++i) v.c[i] = char('a' + i);
    s.store(v);
    Small out{};
    CHECK(s.try_load(out));
    CHECK_EQ(out.c[12], 'm');
}

TEST(readers_never_see_torn_values) {
    Seqlock<TopOfBook> s(TopOfBook::make(0));
    s.load();  // fail fast if unimplemented
    const std::uint64_t kWrites = testing::stress(2'000'000);
    std::atomic<bool> done{false};
    std::atomic<std::uint64_t> torn{0}, backwards{0}, reads{0};

    std::vector<std::thread> readers;
    for (int r = 0; r < 3; ++r) {
        readers.emplace_back([&] {
            std::uint64_t last = 0;
            while (!done.load(std::memory_order_relaxed)) {
                TopOfBook t = s.load();
                if (!t.consistent()) ++torn;
                if (t.seq < last) ++backwards;  // values never go back in time
                last = t.seq;
                ++reads;
            }
        });
    }
    std::thread writer([&] {
        for (std::uint64_t i = 1; i <= kWrites; ++i) s.store(TopOfBook::make(i));
    });
    writer.join();
    done = true;
    for (auto& t : readers) t.join();

    CHECK_EQ(torn.load(), 0u);
    CHECK_EQ(backwards.load(), 0u);
    CHECK(reads.load() > 0u);
    CHECK_EQ(s.load().seq, kWrites);
    CHECK_EQ(s.version(), kWrites);
}

TEST(writer_is_never_blocked_by_readers) {
    // Readers spin as hard as they can; the writer must still finish its
    // stores (a seqlock writer never waits for readers).
    Seqlock<std::uint64_t> s;
    s.load();
    std::atomic<bool> done{false};
    std::vector<std::thread> readers;
    for (int r = 0; r < 3; ++r)
        readers.emplace_back([&] {
            std::uint64_t sink = 0;
            while (!done.load(std::memory_order_relaxed)) sink += s.load();
            (void)sink;
        });
    const std::uint64_t kWrites = testing::stress(1'000'000);
    for (std::uint64_t i = 1; i <= kWrites; ++i) s.store(i);
    done = true;
    for (auto& t : readers) t.join();
    CHECK_EQ(s.load(), kWrites);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
