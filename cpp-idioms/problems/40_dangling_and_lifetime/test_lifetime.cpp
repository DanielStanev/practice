#include "lifetime.hpp"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "testing.hpp"

using namespace idioms;

namespace {
template <class Arg>
concept FirstWordCallable = requires(Arg&& a) { first_word(std::forward<Arg>(a)); };

Instrument make_instrument() { return Instrument("A_SYMBOL_LONG_ENOUGH_TO_LIVE_ON_THE_HEAP"); }
std::string make_string(const char* s) { return s; }
}  // namespace

TEST(first_word_values) {
    std::string s = "  BUY 100 AAPL";
    CHECK_EQ(first_word(s), std::string_view("BUY"));
    CHECK_EQ(first_word("SELL 5"), std::string_view("SELL"));
    std::string blank = "   ";
    CHECK_EQ(first_word(blank), std::string_view(""));
}

TEST(first_word_rejects_temporaries) {
    CHECK_TRAIT(FirstWordCallable<std::string&>);
    CHECK_TRAIT(FirstWordCallable<const std::string&>);
    CHECK_TRAIT(FirstWordCallable<const char*>);
    CHECK_TRAIT(!FirstWordCallable<std::string>);
    CHECK_TRAIT(!FirstWordCallable<std::string&&>);
}

TEST(instrument_symbol_ref_qualified) {
    Instrument i("MSFT");
    CHECK_TRAIT(std::is_same_v<decltype(i.symbol()), const std::string&>);
    CHECK_TRAIT(std::is_same_v<decltype(make_instrument().symbol()), std::string>);
    std::size_t n = 0;
    for (char c : make_instrument().symbol()) n += (c == '_');  // safe: iterates a returned value
    CHECK_EQ(n, std::size_t{8});
    CHECK_EQ(i.symbol(), std::string("MSFT"));
}

TEST(tag_owns_its_name) {
    auto t = std::make_unique<Tag>(make_string("strategy-alpha-with-a-long-name"));
    CHECK_EQ(t->name(), std::string_view("strategy-alpha-with-a-long-name"));
}

TEST(larger_never_dangles) {
    const auto& r = larger(make_string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), make_string("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"));
    std::string after = make_string("cccccccccccccccccccccccccccccccccccc");  // reuse freed memory, if any
    CHECK_EQ(r, std::string("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"));
    std::string x = "x", y = "y";
    CHECK_EQ(larger(x, y), std::string("y"));
}

TEST(adders_capture_by_value) {
    auto adders = make_adders();
    CHECK_EQ(adders[0](1), 11);
    CHECK_EQ(adders[2](1), 31);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
