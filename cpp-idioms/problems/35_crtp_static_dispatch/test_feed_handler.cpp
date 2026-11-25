#include "feed_handler.hpp"

#include <type_traits>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace feed_test {
struct AddCounter : FeedHandler<AddCounter> {
    long adds = 0;
    void on_add(const Add&) { ++adds; }
};
struct TradeAndCancel : FeedHandler<TradeAndCancel> {
    long trades = 0, cancels = 0;
    void on_trade(const Trade&) { ++trades; }
    void on_cancel(const Cancel&) { ++cancels; }
};
struct Typo : FeedHandler<Typo> {
    long trades = 0;
    void on_trades(const Trade&) { ++trades; }  // never called
};
struct StrictGood : StrictFeedHandler<StrictGood> {
    long trades = 0;
    void on_trade(const Trade&) { ++trades; }
};
struct StrictBad : StrictFeedHandler<StrictBad> {};

std::vector<Message> feed() {
    std::vector<Message> v;
    v.push_back({'A', {1, 100.0, 10}, {}, {}});
    v.push_back({'T', {}, {}, {100.0, 5}});
    v.push_back({'A', {2, 101.0, 20}, {}, {}});
    v.push_back({'X', {}, {1}, {}});
    v.push_back({'T', {}, {}, {101.0, 15}});
    v.push_back({'Q', {}, {}, {}});
    return v;
}
}  // namespace feed_test
using namespace feed_test;

TEST(no_virtual_functions) {
    CHECK_TRAIT(!std::is_polymorphic_v<FeedHandler<AddCounter>>);
    CHECK_TRAIT(!std::is_polymorphic_v<AddCounter>);
    CHECK_TRAIT(!std::is_polymorphic_v<VwapTracker>);
}

TEST(dispatch_to_derived_or_default) {
    AddCounter a;
    a.process_all(feed());
    CHECK_EQ(a.adds, 2L);
    CHECK_EQ(a.ignored(), 4L);  // 2 trades, 1 cancel, 1 unknown
    TradeAndCancel tc;
    tc.process_all(feed());
    CHECK_EQ(tc.trades, 2L);
    CHECK_EQ(tc.cancels, 1L);
    CHECK_EQ(tc.ignored(), 3L);
}

TEST(typo_falls_back_silently) {
    Typo t;
    t.process_all(feed());
    CHECK_EQ(t.trades, 0L);
    CHECK_EQ(t.ignored(), 6L);
}

TEST(detect_override) {
    CHECK_TRAIT(overrides_on_trade_v<TradeAndCancel>);
    CHECK_TRAIT(overrides_on_trade_v<VwapTracker>);
    CHECK_TRAIT(!overrides_on_trade_v<AddCounter>);
    CHECK_TRAIT(!overrides_on_trade_v<Typo>);
}

TEST(strict_handler) {
    StrictGood g;
    g.process_all(feed());
    CHECK_EQ(g.trades, 2L);
    // Checked by compiling: StrictBad{}.process(m) would trigger the static_assert,
    // so we only check the good one here and the trait above.
    CHECK_TRAIT(!overrides_on_trade_v<StrictBad>);
}

TEST(vwap_tracker) {
    VwapTracker v;
    CHECK_EQ(v.vwap(), 0.0);
    v.process_all(feed());
    CHECK_NEAR(v.vwap(), (100.0 * 5 + 101.0 * 15) / 20.0, 1e-12);
    CHECK_EQ(v.ignored(), 4L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
