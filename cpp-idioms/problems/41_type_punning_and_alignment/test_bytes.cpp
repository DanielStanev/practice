#include "bytes.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
std::vector<std::byte> bytes(std::initializer_list<int> v) {
    std::vector<std::byte> out;
    for (int b : v) out.push_back(static_cast<std::byte>(b));
    return out;
}
template <auto V>
struct Const {
    static constexpr auto value = V;
};
template <class L>
concept ConstexprOk = requires { Const<L{}()>::value; };
template <class L>
bool is_constexpr(L) {
    return ConstexprOk<L>;
}
}  // namespace

TEST(load_store_any_alignment) {
    alignas(16) std::byte buf[32] = {};
    for (std::size_t off = 0; off < 9; ++off) {
        store<std::uint64_t>(buf + off, 0x0102030405060708ULL + off);
        CHECK_EQ(load<std::uint64_t>(buf + off), 0x0102030405060708ULL + off);
        store<double>(buf + off, 1.5 * static_cast<double>(off));
        CHECK_EQ(load<double>(buf + off), 1.5 * static_cast<double>(off));
    }
}

TEST(byteswap_values) {
    CHECK_EQ(byteswap(std::uint16_t{0x1234}), std::uint16_t{0x3412});
    CHECK_EQ(byteswap(std::uint32_t{0x11223344u}), std::uint32_t{0x44332211u});
    CHECK_EQ(byteswap(std::uint64_t{0x0102030405060708ULL}), std::uint64_t{0x0807060504030201ULL});
    CHECK(is_constexpr([] { return byteswap(std::uint32_t{1}); }));
}

TEST(endian_loads) {
    auto b = bytes({0x00, 0x12, 0x34, 0x56, 0x78});
    CHECK_EQ(load_be<std::uint32_t>(b.data() + 1), std::uint32_t{0x12345678u});
    CHECK_EQ(load_le<std::uint32_t>(b.data() + 1), std::uint32_t{0x78563412u});
    CHECK_EQ(load_be<std::uint16_t>(b.data() + 3), std::uint16_t{0x5678});
}

TEST(double_bits_roundtrip) {
    CHECK_EQ(double_bits(1.0), std::uint64_t{0x3FF0000000000000ULL});
    CHECK_EQ(double_bits(-0.0), std::uint64_t{0x8000000000000000ULL});
    CHECK_EQ(bits_double(0x4000000000000000ULL), 2.0);
    CHECK_EQ(bits_double(double_bits(123.456)), 123.456);
    CHECK(is_constexpr([] { return double_bits(0.5); }));
}

TEST(parse_header_at_odd_offset) {
    // offset 3: u16 len=0x0021, u8 type='A', u64 seq=0x0000000100000002, i32 price=-1500
    auto b = bytes({0xEE, 0xEE, 0xEE, 0x00, 0x21, 'A', 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xFF, 0xFF,
                    0xFA, 0x24});
    Header h = parse_header(b.data(), 3);
    CHECK_EQ(h.length, std::uint16_t{0x21});
    CHECK_EQ(h.type, std::uint8_t{'A'});
    CHECK_EQ(h.seq, std::uint64_t{0x0000000100000002ULL});
    CHECK_EQ(h.price_ticks, -1500);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
