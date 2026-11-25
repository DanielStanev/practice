// Reference solution for problem 34.
#pragma once

#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace idioms {

template <class D>
class LessThanComparable {
    LessThanComparable() = default;
    friend D;

    friend bool operator>(const D& a, const D& b) { return b < a; }
    friend bool operator<=(const D& a, const D& b) { return !(b < a); }
    friend bool operator>=(const D& a, const D& b) { return !(a < b); }
    friend bool operator==(const D& a, const D& b) { return !(a < b) && !(b < a); }
    friend bool operator!=(const D& a, const D& b) { return !(a == b); }
};

template <class D>
class InstanceCounter {
public:
    static long created() { return created_; }
    static long alive() { return alive_; }

protected:
    InstanceCounter() noexcept { ++created_, ++alive_; }
    InstanceCounter(const InstanceCounter&) noexcept { ++created_, ++alive_; }
    InstanceCounter(InstanceCounter&&) noexcept { ++created_, ++alive_; }
    InstanceCounter& operator=(const InstanceCounter&) = default;
    InstanceCounter& operator=(InstanceCounter&&) = default;
    ~InstanceCounter() { --alive_; }

private:
    static inline long created_ = 0;
    static inline long alive_ = 0;
};

template <class Base, class Derived>
struct Cloneable : Base {
    using Base::Base;
    std::unique_ptr<Base> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

template <class D>
class Printable {
public:
    std::string to_string() const {
        std::ostringstream os;
        os << D::name() << '{';
        static_cast<const D&>(*this).fields(os);
        os << '}';
        return os.str();
    }
};

}  // namespace idioms
