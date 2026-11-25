#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "../09_latency_histogram/latency_histogram.hpp"
#include "../15_itch_parser/itch_parser.hpp"
#include "../17_l2_order_book/order_book.hpp"

namespace hft::t2t {

using book::Side;

struct OrderRequest {
    std::uint64_t client_order_id;
    Side side;
    std::int64_t price;
    std::uint32_t qty;
    friend bool operator==(const OrderRequest&, const OrderRequest&) = default;
};

class TokenBucket {
public:
    static constexpr std::uint64_t kNs = 1'000'000'000;

    TokenBucket(std::uint32_t rate_per_sec, std::uint32_t burst)
        : rate_(rate_per_sec), capacity_(std::uint64_t(burst) * kNs), level_(capacity_) {}

    bool try_consume(std::uint64_t now_ns) {
        refill(now_ns);
        if (level_ < kNs) return false;
        level_ -= kNs;
        return true;
    }

    std::uint32_t available(std::uint64_t now_ns) {
        refill(now_ns);
        return static_cast<std::uint32_t>(level_ / kNs);
    }

private:
    void refill(std::uint64_t now_ns) {
        if (!started_) {
            started_ = true;
            last_ = now_ns;
            return;
        }
        if (now_ns <= last_) return;
        std::uint64_t elapsed = now_ns - last_;
        last_ = now_ns;
        // elapsed * rate tokens-ns; cap before multiplying to avoid overflow
        std::uint64_t add = elapsed >= capacity_ ? capacity_ : elapsed * rate_;
        level_ = std::min(capacity_, level_ + std::min(add, capacity_));
    }

    std::uint64_t rate_;
    std::uint64_t capacity_;
    std::uint64_t level_;
    std::uint64_t last_ = 0;
    bool started_ = false;
};

struct RiskLimits {
    std::uint32_t max_order_qty;
    std::int64_t max_position;      // absolute position limit, in shares
    std::uint32_t max_orders_per_sec;
    std::uint32_t burst;
    std::int64_t price_collar;      // max |price - reference|
};

enum class RiskResult { Accepted, MaxOrderQty, PriceCollar, NoReferencePrice, MaxPosition, RateLimited };

class RiskChecker {
public:
    explicit RiskChecker(RiskLimits l) : limits_(l), bucket_(l.max_orders_per_sec, l.burst) {}

    RiskResult check(const OrderRequest& o, std::optional<std::int64_t> reference, std::uint64_t now_ns) {
        if (o.qty == 0 || o.qty > limits_.max_order_qty) return RiskResult::MaxOrderQty;
        if (!reference) return RiskResult::NoReferencePrice;
        std::int64_t dist = o.price > *reference ? o.price - *reference : *reference - o.price;
        if (dist > limits_.price_collar) return RiskResult::PriceCollar;
        std::int64_t signed_qty = o.side == Side::Buy ? std::int64_t(o.qty) : -std::int64_t(o.qty);
        std::int64_t after = position_ + signed_qty;
        if (after > limits_.max_position || after < -limits_.max_position) return RiskResult::MaxPosition;
        if (!bucket_.try_consume(now_ns)) return RiskResult::RateLimited;
        return RiskResult::Accepted;
    }

    void on_fill(Side side, std::uint32_t qty) {
        position_ += side == Side::Buy ? std::int64_t(qty) : -std::int64_t(qty);
    }
    std::int64_t position() const { return position_; }

private:
    RiskLimits limits_;
    TokenBucket bucket_;
    std::int64_t position_ = 0;
};

inline constexpr std::size_t kEnterOrderSize = 19;

inline std::size_t encode_enter_order(const OrderRequest& o, std::span<std::byte> out) {
    if (out.size() < kEnterOrderSize) return 0;
    std::byte* p = out.data();
    p[0] = std::byte{'O'};
    itch::store_be<std::uint64_t>(p + 1, o.client_order_id);
    p[9] = std::byte{static_cast<unsigned char>(o.side == Side::Buy ? 'B' : 'S')};
    itch::store_be<std::uint32_t>(p + 10, o.qty);
    itch::store_be<std::uint32_t>(p + 14, static_cast<std::uint32_t>(o.price));
    p[18] = std::byte{'I'};
    return kEnterOrderSize;
}

struct StrategyParams {
    double threshold;    // in (0, 1]
    std::uint32_t clip;  // order size
};

class ImbalanceStrategy {
public:
    explicit ImbalanceStrategy(StrategyParams p) : p_(p) {}

    std::optional<OrderRequest> on_book(const book::OrderBook& b, std::uint64_t next_id) {
        auto bid = b.best_bid();
        auto ask = b.best_ask();
        int signal = 0;
        if (bid && ask) {
            double bq = double(bid->qty), aq = double(ask->qty);
            double imb = (bq - aq) / (bq + aq);
            if (imb >= p_.threshold) signal = 1;
            else if (imb <= -p_.threshold) signal = -1;
        }
        int prev = last_;
        last_ = signal;
        if (signal == 0 || signal == prev) return std::nullopt;
        if (signal > 0) return OrderRequest{next_id, Side::Buy, ask->price, p_.clip};
        return OrderRequest{next_id, Side::Sell, bid->price, p_.clip};
    }

private:
    StrategyParams p_;
    int last_ = 0;
};

struct Config {
    std::string stock;
    StrategyParams strategy;
    RiskLimits risk;
};

class TickToTrade {
    struct Handler {
        TickToTrade* t;
        void on_add(const itch::AddOrder& m) {
            t->builder_.on_add(m);
            t->after_update();
        }
        void on_execute(const itch::OrderExecuted& m) {
            t->builder_.on_execute(m);
            t->after_update();
        }
        void on_cancel(const itch::OrderCancel& m) {
            t->builder_.on_cancel(m);
            t->after_update();
        }
        void on_delete(const itch::OrderDelete& m) {
            t->builder_.on_delete(m);
            t->after_update();
        }
        void on_replace(const itch::OrderReplace& m) {
            t->builder_.on_replace(m);
            t->after_update();
        }
    };

public:
    explicit TickToTrade(Config cfg)
        : cfg_(std::move(cfg)),
          builder_(book_, cfg_.stock),
          strategy_(cfg_.strategy),
          risk_(cfg_.risk),
          latency_(10'000'000'000ull, 7) {
        wire_out_.reserve(1 << 16);
    }

    void on_bytes(std::span<const std::byte> chunk, std::uint64_t rx_ts_ns) {
        t_start_ = steady_now_ns();
        rx_ts_ = rx_ts_ns;
        Handler h{this};
        if (pending_.empty()) {
            auto r = itch::parse(chunk, h);
            pending_.insert(pending_.end(), chunk.begin() + std::ptrdiff_t(r.consumed), chunk.end());
        } else {
            pending_.insert(pending_.end(), chunk.begin(), chunk.end());
            auto r = itch::parse(std::span<const std::byte>(pending_), h);
            pending_.erase(pending_.begin(), pending_.begin() + std::ptrdiff_t(r.consumed));
        }
    }

    void on_fill(Side side, std::uint32_t qty) { risk_.on_fill(side, qty); }

    const std::vector<std::byte>& wire_out() const { return wire_out_; }
    void clear_wire_out() { wire_out_.clear(); }
    const std::vector<OrderRequest>& sent() const { return sent_; }
    const std::vector<std::pair<OrderRequest, RiskResult>>& rejected() const { return rejected_; }
    const LatencyHistogram& latency() const { return latency_; }
    std::int64_t position() const { return risk_.position(); }
    const book::OrderBook& book() const { return book_; }

private:
    void after_update() {
        auto order = strategy_.on_book(book_, next_id_);
        if (!order) return;
        ++next_id_;
        std::optional<std::int64_t> ref;
        auto bid = book_.best_bid();
        auto ask = book_.best_ask();
        if (bid && ask) ref = (bid->price + ask->price) / 2;
        RiskResult rr = risk_.check(*order, ref, rx_ts_);
        if (rr != RiskResult::Accepted) {
            rejected_.emplace_back(*order, rr);
            return;
        }
        std::size_t off = wire_out_.size();
        wire_out_.resize(off + kEnterOrderSize);
        encode_enter_order(*order, std::span<std::byte>(wire_out_).subspan(off));
        sent_.push_back(*order);
        latency_.record(steady_now_ns() - t_start_);
    }

    Config cfg_;
    book::OrderBook book_;
    book::BookBuilder builder_;
    ImbalanceStrategy strategy_;
    RiskChecker risk_;
    LatencyHistogram latency_;
    std::vector<std::byte> pending_;
    std::vector<std::byte> wire_out_;
    std::vector<OrderRequest> sent_;
    std::vector<std::pair<OrderRequest, RiskResult>> rejected_;
    std::uint64_t next_id_ = 1;
    std::uint64_t t_start_ = 0;
    std::uint64_t rx_ts_ = 0;
};

}  // namespace hft::t2t
