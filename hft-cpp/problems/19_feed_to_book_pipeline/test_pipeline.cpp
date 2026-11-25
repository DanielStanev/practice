#include "pipeline.hpp"

#include <sched.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <map>
#include <random>
#include <span>
#include <thread>
#include <vector>

#include "itch_wire.hpp"
#include "testing.hpp"

using hft::pipeline::Config;
using hft::pipeline::Pipeline;
using hft::pipeline::TopOfBook;

namespace {

// Independent model of the book: replays the same stream and records the
// expected top of book after every event.
struct Model {
    struct O {
        bool buy;
        std::int64_t px;
        std::uint64_t qty;
    };
    std::map<std::uint64_t, O> orders;
    std::map<std::int64_t, std::uint64_t> bids, asks;  // price -> total qty
    std::vector<TopOfBook> tops{TopOfBook{}};

    void level_add(const O& o, std::int64_t delta) {
        auto& side = o.buy ? bids : asks;
        auto& q = side[o.px];
        q = std::uint64_t(std::int64_t(q) + delta);
        if (q == 0) side.erase(o.px);
    }
    void snapshot() {
        TopOfBook t;
        t.sequence = tops.size();
        if (!bids.empty()) {
            t.has_bid = true;
            t.bid_px = bids.rbegin()->first;
            t.bid_qty = bids.rbegin()->second;
        }
        if (!asks.empty()) {
            t.has_ask = true;
            t.ask_px = asks.begin()->first;
            t.ask_qty = asks.begin()->second;
        }
        tops.push_back(t);
    }
    void insert(std::uint64_t ref, O o) {
        orders.emplace(ref, o);
        level_add(o, std::int64_t(o.qty));
    }
    void add(std::uint64_t ref, bool buy, std::int64_t px, std::uint64_t q, bool ours) {
        if (ours && q > 0) insert(ref, O{buy, px, q});
        snapshot();
    }
    void reduce(std::uint64_t ref, std::uint64_t q) {
        auto it = orders.find(ref);
        if (it != orders.end()) {
            std::uint64_t take = std::min(q, it->second.qty);
            level_add(it->second, -std::int64_t(take));
            it->second.qty -= take;
            if (it->second.qty == 0) orders.erase(it);
        }
        snapshot();
    }
    void del(std::uint64_t ref) {
        auto it = orders.find(ref);
        if (it != orders.end()) {
            level_add(it->second, -std::int64_t(it->second.qty));
            orders.erase(it);
        }
        snapshot();
    }
    void replace(std::uint64_t ref, std::uint64_t nref, std::uint64_t q, std::int64_t px) {
        auto it = orders.find(ref);
        if (it != orders.end() && !orders.count(nref)) {
            bool buy = it->second.buy;
            level_add(it->second, -std::int64_t(it->second.qty));
            orders.erase(it);
            if (q > 0) insert(nref, O{buy, px, q});
        }
        snapshot();
    }
};

struct Stream {
    wire::Writer w;
    Model model;
};

// Random but realistic flow for AAPL with MSFT noise interleaved.
Stream make_stream(int n, unsigned seed) {
    Stream s;
    std::mt19937_64 rng(seed);
    std::vector<std::uint64_t> live;
    std::uint64_t next = 1;
    for (int i = 0; i < n; ++i) {
        int op = int(rng() % 10);
        std::uint64_t ts = std::uint64_t(i);
        if (op < 4 || live.empty()) {
            bool ours = rng() % 4 != 0;
            bool buy = rng() % 2;
            std::uint32_t px = buy ? 1000000 - std::uint32_t(rng() % 50) * 100 : 1000100 + std::uint32_t(rng() % 50) * 100;
            std::uint32_t q = 1 + std::uint32_t(rng() % 500);
            s.w.add_order(ts, next, buy ? 'B' : 'S', q, ours ? "AAPL" : "MSFT", px);
            s.model.add(next, buy, px, q, ours);
            live.push_back(next++);
            continue;
        }
        std::size_t k = rng() % live.size();
        std::uint64_t ref = live[k];
        if (op < 6) {
            std::uint32_t q = 1 + std::uint32_t(rng() % 200);
            s.w.executed(ts, ref, q, ts);
            s.model.reduce(ref, q);
        } else if (op < 7) {
            std::uint32_t q = 1 + std::uint32_t(rng() % 200);
            s.w.cancel(ts, ref, q);
            s.model.reduce(ref, q);
        } else if (op < 9) {
            s.w.del(ts, ref);
            s.model.del(ref);
            live[k] = live.back();
            live.pop_back();
        } else {
            std::uint32_t px = 990000 + std::uint32_t(rng() % 200) * 100;
            std::uint32_t q = 1 + std::uint32_t(rng() % 500);
            s.w.replace(ts, ref, next, q, px);
            s.model.replace(ref, next, q, px);
            live[k] = next++;
        }
    }
    return s;
}

void feed_in_chunks(Pipeline& p, const std::vector<std::byte>& bytes, unsigned seed) {
    std::mt19937 rng(seed);
    std::size_t pos = 0;
    while (pos < bytes.size()) {
        std::size_t n = std::min<std::size_t>(1 + rng() % 1500, bytes.size() - pos);
        p.on_bytes(std::span<const std::byte>(bytes.data() + pos, n));
        pos += n;
    }
}

}  // namespace

TEST(start_and_stop_without_data) {
    Pipeline p(Config{"AAPL"});
    p.start();
    p.stop();
    CHECK_EQ(p.top().sequence, 0u);
    CHECK_EQ(p.events_applied(), 0u);
    p.stop();  // idempotent
}

TEST(destructor_joins_thread) {
    {
        Pipeline p(Config{"AAPL"});
        p.start();
    }
    CHECK(true);  // reaching here without std::terminate is the test
}

TEST(single_message_reaches_book) {
    wire::Writer w;
    w.add_order(1, 7, 'B', 100, "AAPL", 1234500);
    Pipeline p(Config{"AAPL"});
    p.start();
    p.on_bytes(std::span<const std::byte>(w.bytes.data(), w.bytes.size()));
    p.stop();
    CHECK_EQ(p.events_applied(), 1u);
    TopOfBook t = p.top();
    CHECK(t.has_bid);
    CHECK_FALSE(t.has_ask);
    CHECK_EQ(t.bid_px, 1234500);
    CHECK_EQ(t.bid_qty, 100u);
    CHECK_EQ(t.sequence, 1u);
}

TEST(messages_split_across_chunks) {
    wire::Writer w;
    w.add_order(1, 1, 'S', 10, "AAPL", 500);
    w.add_order(2, 2, 'S', 20, "AAPL", 400);
    Pipeline p(Config{"AAPL"});
    p.start();
    for (std::size_t i = 0; i < w.bytes.size(); ++i)  // one byte at a time
        p.on_bytes(std::span<const std::byte>(w.bytes.data() + i, 1));
    p.stop();
    CHECK_EQ(p.events_applied(), 2u);
    CHECK_EQ(p.top().ask_px, 400);
}

TEST(final_book_matches_model) {
    Stream s = make_stream(int(testing::stress(200'000)), 1);
    Pipeline p(Config{"AAPL"});
    p.start();
    feed_in_chunks(p, s.w.bytes, 2);
    p.stop();
    std::uint64_t n = s.model.tops.size() - 1;
    CHECK_EQ(p.events_applied(), n);
    CHECK(p.top() == s.model.tops.back());
    CHECK_EQ(p.book().order_count(), s.model.orders.size());
}

TEST(concurrent_readers_see_exact_snapshots) {
    // Readers poll top() while the book thread is updating it. Every
    // snapshot must be exactly the model's top of book at that sequence
    // number (no torn reads) and sequence numbers never go backwards.
    Stream s = make_stream(int(testing::stress(200'000)), 3);
    Pipeline p(Config{"AAPL"});
    p.start();
    std::atomic<bool> done{false};
    std::atomic<std::uint64_t> bad{0}, backwards{0}, observed{0};
    std::vector<std::thread> readers;
    for (int r = 0; r < 2; ++r)
        readers.emplace_back([&] {
            std::uint64_t last = 0;
            while (!done.load(std::memory_order_relaxed)) {
                TopOfBook t = p.top();
                if (t.sequence >= s.model.tops.size() || !(t == s.model.tops[t.sequence])) ++bad;
                if (t.sequence < last) ++backwards;
                last = t.sequence;
                ++observed;
            }
        });
    feed_in_chunks(p, s.w.bytes, 4);
    p.stop();
    done = true;
    for (auto& t : readers) t.join();
    CHECK_EQ(bad.load(), 0u);
    CHECK_EQ(backwards.load(), 0u);
    CHECK(observed.load() > 0u);
    CHECK(p.top() == s.model.tops.back());
}

TEST(restart_after_stop) {
    wire::Writer a, b;
    a.add_order(1, 1, 'B', 10, "AAPL", 100);
    b.add_order(2, 2, 'B', 10, "AAPL", 200);
    Pipeline p(Config{"AAPL"});
    p.start();
    p.on_bytes(std::span<const std::byte>(a.bytes.data(), a.bytes.size()));
    p.stop();
    p.start();
    p.on_bytes(std::span<const std::byte>(b.bytes.data(), b.bytes.size()));
    p.stop();
    CHECK_EQ(p.events_applied(), 2u);
    CHECK_EQ(p.top().bid_px, 200);
}

TEST(pin_thread) {
    cpu_set_t allowed;
    CPU_ZERO(&allowed);
    sched_getaffinity(0, sizeof allowed, &allowed);
    int cpu = 0;
    while (cpu < CPU_SETSIZE && !CPU_ISSET(cpu, &allowed)) ++cpu;
    bool ok = false, bad_negative = true, bad_huge = true;
    std::thread t([&] {
        ok = hft::pipeline::pin_current_thread(cpu);
        bad_negative = hft::pipeline::pin_current_thread(-1);
        bad_huge = hft::pipeline::pin_current_thread(CPU_SETSIZE + 5);
    });
    t.join();
    CHECK(ok);
    CHECK_FALSE(bad_negative);
    CHECK_FALSE(bad_huge);
}

TEST(pinned_book_thread_works) {
    cpu_set_t allowed;
    CPU_ZERO(&allowed);
    sched_getaffinity(0, sizeof allowed, &allowed);
    int cpu = CPU_SETSIZE - 1;
    while (cpu > 0 && !CPU_ISSET(cpu, &allowed)) --cpu;  // highest allowed CPU
    Stream s = make_stream(5000, 9);
    Pipeline p(Config{"AAPL", cpu});
    p.start();
    feed_in_chunks(p, s.w.bytes, 10);
    p.stop();
    CHECK(p.top() == s.model.tops.back());
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
