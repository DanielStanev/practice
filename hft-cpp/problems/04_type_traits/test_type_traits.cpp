#include "type_traits.hpp"

#include <cstdint>
#include <string>
#include <type_traits>

#include "testing.hpp"

namespace m = hft::meta;

namespace {

struct AddOrder {
    static constexpr char kType = 'A';
    std::uint64_t id;
    std::int64_t price;
    std::uint32_t qty;
};
struct Cancel {
    static constexpr char kType = 'X';
    std::uint64_t id;
};
struct Trade {
    static constexpr char kType = 'P';
    std::uint64_t id;
    std::int64_t price;
    std::uint32_t qty;
    std::uint64_t match;
};
struct NoTypeTag {
    int x;
};
struct NotTrivial {
    [[maybe_unused]] static constexpr char kType = 'Z';
    std::string s;
};

using Messages = m::TypeList<AddOrder, Cancel, Trade>;

}  // namespace

TEST(is_same) {
    CHECK_TRAIT(m::is_same_v<int, int>);
    CHECK_TRAIT(!m::is_same_v<int, long>);
    CHECK_TRAIT(!m::is_same_v<int, const int>);
    CHECK_TRAIT(!m::is_same_v<int, int&>);
}

TEST(remove_cvref) {
    CHECK_TRAIT(m::is_same_v<m::remove_cvref_t<const int&>, int>);
    CHECK_TRAIT(m::is_same_v<m::remove_cvref_t<volatile int&&>, int>);
    CHECK_TRAIT(m::is_same_v<m::remove_cvref_t<const volatile int>, int>);
    CHECK_TRAIT(m::is_same_v<m::remove_cvref_t<int*>, int*>);
    CHECK_TRAIT(m::is_same_v<m::remove_cvref_t<const int*>, const int*>);  // pointee keeps const
    CHECK_TRAIT(m::is_same_v<m::remove_cvref_t<int* const&>, int*>);
}

TEST(conditional) {
    CHECK_TRAIT(m::is_same_v<m::conditional_t<true, int, double>, int>);
    CHECK_TRAIT(m::is_same_v<m::conditional_t<false, int, double>, double>);
    // Classic use: pick the smallest index type that fits.
    constexpr std::size_t kCapacity = 1000;
    using Index = m::conditional_t<(kCapacity < 256), std::uint8_t,
                                   m::conditional_t<(kCapacity < 65536), std::uint16_t, std::uint32_t>>;
    CHECK_TRAIT(m::is_same_v<Index, std::uint16_t>);
}

TEST(list_size) {
    CHECK_EQ(m::list_size_v<m::TypeList<>>, 0u);
    CHECK_EQ(m::list_size_v<Messages>, 3u);
}

TEST(type_at) {
    CHECK_TRAIT(m::is_same_v<m::type_at_t<0, Messages>, AddOrder>);
    CHECK_TRAIT(m::is_same_v<m::type_at_t<2, Messages>, Trade>);
}

TEST(index_of_and_contains) {
    CHECK_EQ((m::index_of_v<AddOrder, Messages>), 0);
    CHECK_EQ((m::index_of_v<Trade, Messages>), 2);
    CHECK_EQ((m::index_of_v<int, Messages>), -1);
    CHECK_EQ((m::index_of_v<int, m::TypeList<int, int>>), 0);  // first match
    CHECK_TRAIT(m::contains_v<Cancel, Messages>);
    CHECK_TRAIT(!m::contains_v<NoTypeTag, Messages>);
}

TEST(push_front) {
    using L = m::push_front_t<int, Messages>;
    CHECK_EQ(m::list_size_v<L>, 4u);
    CHECK_TRAIT(m::is_same_v<m::type_at_t<0, L>, int>);
    CHECK_TRAIT(m::is_same_v<m::type_at_t<1, L>, AddOrder>);
}

TEST(max_size) {
    CHECK_EQ(m::max_size_v<Messages>, sizeof(Trade));
    CHECK_EQ((m::max_size_v<m::TypeList<char, std::uint64_t, std::uint16_t>>), 8u);
}

TEST(power_of_two_helpers) {
    constexpr bool a = m::is_power_of_two(64);  // forces compile-time evaluation
    CHECK(a);
    CHECK_FALSE(m::is_power_of_two(0));
    CHECK_FALSE(m::is_power_of_two(96));
    CHECK(m::is_power_of_two(1ull << 63));

    constexpr std::uint64_t n = m::next_power_of_two(1000);
    CHECK_EQ(n, 1024u);
    CHECK_EQ(m::next_power_of_two(0), 1u);
    CHECK_EQ(m::next_power_of_two(1), 1u);
    CHECK_EQ(m::next_power_of_two(1024), 1024u);
    CHECK_EQ(m::next_power_of_two(1025), 2048u);
    CHECK_EQ(m::next_power_of_two((1ull << 62) + 1), 1ull << 63);

    constexpr int l = m::log2_floor(1);
    CHECK_EQ(l, 0);
    CHECK_EQ(m::log2_floor(1023), 9);
    CHECK_EQ(m::log2_floor(1024), 10);
    CHECK_EQ(m::log2_floor(~0ull), 63);
}

TEST(pow10_table_is_compile_time) {
    constexpr auto t = m::pow10_table<20>();
    CHECK_EQ(t.size(), 20u);
    CHECK_EQ(t[0], 1u);
    CHECK_EQ(t[4], 10000u);
    CHECK_EQ(t[19], 10000000000000000000ull);
}

TEST(count_digits) {
    constexpr int d = m::count_digits(12345);
    CHECK_EQ(d, 5);
    CHECK_EQ(m::count_digits(0), 1);
    CHECK_EQ(m::count_digits(9), 1);
    CHECK_EQ(m::count_digits(10), 2);
    CHECK_EQ(m::count_digits(999999), 6);
    CHECK_EQ(m::count_digits(1000000), 7);
    CHECK_EQ(m::count_digits(~0ull), 20);
}

TEST(fnv1a_hash) {
    // 64-bit FNV-1a reference values.
    constexpr std::uint64_t empty = m::fnv1a("");
    CHECK_EQ(empty, 14695981039346656037ull);
    CHECK_EQ(m::fnv1a("a"), 0xaf63dc4c8601ec8cull);
    CHECK_EQ(m::fnv1a("foobar"), 0x85944171f73967e8ull);
    CHECK(m::fnv1a("AAPL") != m::fnv1a("MSFT"));
}

TEST(wire_message_concept) {
    CHECK_TRAIT(m::WireMessage<AddOrder>);
    CHECK_TRAIT(m::WireMessage<Trade>);
    CHECK_TRAIT(!m::WireMessage<NoTypeTag>);
    CHECK_TRAIT(!m::WireMessage<NotTrivial>);
    CHECK_TRAIT(!m::WireMessage<int>);
}

TEST(dispatch_on_runtime_tag) {
    int seen_size = 0;
    bool found = m::dispatch<Messages>('X', [&](auto tag) {
        using M = typename decltype(tag)::type;
        seen_size = static_cast<int>(sizeof(M));
    });
    CHECK(found);
    CHECK_EQ(seen_size, static_cast<int>(sizeof(Cancel)));

    found = m::dispatch<Messages>('P', [&](auto tag) {
        using M = typename decltype(tag)::type;
        seen_size = static_cast<int>(sizeof(M));
    });
    CHECK(found);
    CHECK_EQ(seen_size, static_cast<int>(sizeof(Trade)));

    int calls = 0;
    found = m::dispatch<Messages>('?', [&](auto) { ++calls; });
    CHECK_FALSE(found);
    CHECK_EQ(calls, 0);
}

TEST(dispatch_is_constexpr) {
    constexpr bool ok = m::dispatch<Messages>('A', [](auto) {});
    CHECK(ok);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
