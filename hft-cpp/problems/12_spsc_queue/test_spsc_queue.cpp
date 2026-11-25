#include "spsc_queue.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>

#include "testing.hpp"

using hft::SpscQueue;

namespace {

struct Tracked {
    static inline int alive = 0;
    int v;
    explicit Tracked(int x) : v(x) { ++alive; }
    Tracked(const Tracked& o) : v(o.v) { ++alive; }
    Tracked(Tracked&& o) noexcept : v(o.v) { ++alive; }
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) noexcept = default;
    ~Tracked() { --alive; }
};

struct Message {
    std::uint64_t seq;
    std::uint64_t payload[3];
};

}  // namespace

TEST(fifo_single_thread) {
    SpscQueue<int, 8> q;
    CHECK(q.empty());
    CHECK_EQ(q.capacity(), 8u);
    for (int i = 0; i < 5; ++i) CHECK(q.try_push(i));
    CHECK_EQ(q.size_approx(), 5u);
    int out = -1;
    for (int i = 0; i < 5; ++i) {
        CHECK(q.try_pop(out));
        CHECK_EQ(out, i);
    }
    CHECK_FALSE(q.try_pop(out));
    CHECK(q.empty());
}

TEST(uses_full_capacity) {
    SpscQueue<int, 4> q;
    for (int i = 0; i < 4; ++i) CHECK(q.try_push(i));
    CHECK_FALSE(q.try_push(99));  // exactly Capacity elements fit
    int out;
    CHECK(q.try_pop(out));
    CHECK(q.try_push(4));
    CHECK_FALSE(q.try_push(5));
}

TEST(wraps_around_many_times) {
    SpscQueue<std::uint64_t, 4> q;
    std::uint64_t out = 0;
    for (std::uint64_t i = 0; i < 100000; ++i) {
        CHECK(q.try_push(i));
        if (i % 3 == 2) {
            CHECK(q.try_pop(out));
            CHECK(q.try_pop(out));
            CHECK(q.try_pop(out));
            CHECK_EQ(out, i);
        }
    }
}

TEST(front_and_pop_avoid_copies) {
    SpscQueue<std::string, 8> q;
    CHECK(q.front() == nullptr);
    q.try_emplace(100, 'x');
    q.try_emplace("hello");
    std::string* f = q.front();
    CHECK(f != nullptr);
    CHECK_EQ(f->size(), 100u);
    q.pop();
    CHECK_EQ(*q.front(), std::string("hello"));
    q.pop();
    CHECK(q.front() == nullptr);
}

TEST(objects_constructed_and_destroyed_once) {
    Tracked::alive = 0;
    {
        SpscQueue<Tracked, 16> q;
        CHECK_EQ(Tracked::alive, 0);  // no default-constructed slots
        for (int i = 0; i < 10; ++i) q.try_emplace(i);
        CHECK_EQ(Tracked::alive, 10);
        CHECK(q.front() != nullptr);
        q.pop();
        CHECK(q.front() != nullptr);
        q.pop();
        CHECK_EQ(Tracked::alive, 8);
        Tracked out(0);
        q.try_pop(out);
        CHECK_EQ(out.v, 2);
        CHECK_EQ(Tracked::alive, 8);  // 7 queued + out
    }
    // Destructor destroyed the 7 that were still queued.
    CHECK_EQ(Tracked::alive, 0);
}

TEST(no_default_constructor_required) {
    struct NoDefault {
        int x;
        explicit NoDefault(int v) : x(v) {}
        NoDefault() = delete;
    };
    SpscQueue<NoDefault, 4> q;
    q.try_emplace(5);
    CHECK_EQ(q.front()->x, 5);
    q.pop();
}

TEST(head_and_tail_on_separate_cache_lines) {
    // The producer writes tail, the consumer writes head. Sharing a cache
    // line between them ("false sharing") makes every operation a cache miss.
    CHECK(alignof(SpscQueue<int, 8>) >= hft::kCacheLine);
    CHECK(sizeof(SpscQueue<int, 8>) >= 2 * hft::kCacheLine + 8 * sizeof(int));
    CHECK_TRAIT(!std::is_copy_constructible_v<SpscQueue<int, 8>>);
}

TEST(two_threads_preserve_order) {
    auto q = std::make_unique<SpscQueue<Message, 1024>>();
    q->front();  // fail fast if unimplemented
    constexpr std::uint64_t kCount = testing::stress(1'000'000);
    std::thread producer([&] {
        for (std::uint64_t i = 0; i < kCount; ++i) {
            Message m{i, {i * 2, i * 3, i * 5}};
            while (!q->try_push(m)) {
            }
        }
    });
    std::uint64_t expected = 0;
    bool ok = true;
    while (expected < kCount) {
        if (Message* m = q->front()) {
            if (m->seq != expected || m->payload[0] != expected * 2 || m->payload[2] != expected * 5)
                ok = false;
            q->pop();
            ++expected;
        }
    }
    producer.join();
    CHECK(ok);
    CHECK(q->empty());
}

TEST(two_threads_move_only_payload) {
    auto q = std::make_unique<SpscQueue<std::unique_ptr<std::uint64_t>, 64>>();
    q->front();
    constexpr std::uint64_t kCount = testing::stress(200'000);
    std::thread producer([&] {
        for (std::uint64_t i = 0; i < kCount; ++i) {
            auto p = std::make_unique<std::uint64_t>(i);
            while (!q->try_push(std::move(p))) {
            }
        }
    });
    std::uint64_t sum = 0;
    for (std::uint64_t got = 0; got < kCount;) {
        std::unique_ptr<std::uint64_t> p;
        if (q->try_pop(p)) {
            sum += *p;
            ++got;
        }
    }
    producer.join();
    CHECK_EQ(sum, kCount * (kCount - 1) / 2);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
