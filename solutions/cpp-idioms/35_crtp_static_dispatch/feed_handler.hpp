// Reference solution for problem 35.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace idioms {

struct Add {
    std::uint64_t id;
    double price;
    long qty;
};
struct Cancel {
    std::uint64_t id;
};
struct Trade {
    double price;
    long qty;
};
struct Message {
    char type;
    Add add{};
    Cancel cancel{};
    Trade trade{};
};

template <class Derived>
class FeedHandler {
public:
    void process(const Message& m) {
        Derived& d = static_cast<Derived&>(*this);
        switch (m.type) {
            case 'A': d.on_add(m.add); break;
            case 'X': d.on_cancel(m.cancel); break;
            case 'T': d.on_trade(m.trade); break;
            default: d.on_unknown(m); break;
        }
    }
    void process_all(const std::vector<Message>& ms) {
        for (const Message& m : ms) process(m);
    }
    long ignored() const { return ignored_; }

    void on_add(const Add&) { ++ignored_; }
    void on_cancel(const Cancel&) { ++ignored_; }
    void on_trade(const Trade&) { ++ignored_; }
    void on_unknown(const Message&) { ++ignored_; }

private:
    long ignored_ = 0;
};

template <class D>
inline constexpr bool overrides_on_trade_v =
    !std::is_same_v<decltype(&D::on_trade), void (FeedHandler<D>::*)(const Trade&)>;

template <class Derived>
class StrictFeedHandler : public FeedHandler<Derived> {
public:
    void process(const Message& m) {
        static_assert(overrides_on_trade_v<Derived>, "StrictFeedHandler requires Derived::on_trade");
        FeedHandler<Derived>::process(m);
    }
};

class VwapTracker : public FeedHandler<VwapTracker> {
public:
    double vwap() const { return qty_ == 0 ? 0.0 : notional_ / static_cast<double>(qty_); }
    void on_trade(const Trade& t) {
        notional_ += t.price * static_cast<double>(t.qty);
        qty_ += t.qty;
    }

private:
    double notional_ = 0;
    long qty_ = 0;
};

}  // namespace idioms
