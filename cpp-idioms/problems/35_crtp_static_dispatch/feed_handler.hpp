// Problem 35: CRTP static dispatch.
// Read README.md first. Implement everything marked TODO.
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
        (void)m;
        throw std::logic_error("TODO: process");
    }
    void process_all(const std::vector<Message>& ms) {
        (void)ms;
        throw std::logic_error("TODO: process_all");
    }
    long ignored() const { return ignored_; }

    // TODO: default hooks on_add, on_cancel, on_trade, on_unknown.
    void on_trade(const Trade&) { throw std::logic_error("TODO: default on_trade"); }

private:
    long ignored_ = 0;
};

template <class D>
inline constexpr bool overrides_on_trade_v = false;  // TODO

template <class Derived>
class StrictFeedHandler : public FeedHandler<Derived> {
public:
    // TODO: like FeedHandler::process, but refuse to compile unless Derived declares on_trade.
};

class VwapTracker : public FeedHandler<VwapTracker> {
public:
    double vwap() const { throw std::logic_error("TODO: vwap"); }
    // TODO: on_trade
};

}  // namespace idioms
