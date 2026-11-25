// Problem 34: CRTP mixins.
// Read README.md first. Implement everything marked TODO.
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
    // TODO: private constructor, friend D.

    // TODO: implement in terms of D's operator<.
    friend bool operator>(const D&, const D&) { throw std::logic_error("TODO: >"); }
    friend bool operator<=(const D&, const D&) { throw std::logic_error("TODO: <="); }
    friend bool operator>=(const D&, const D&) { throw std::logic_error("TODO: >="); }
    friend bool operator==(const D&, const D&) { throw std::logic_error("TODO: =="); }
    friend bool operator!=(const D&, const D&) { throw std::logic_error("TODO: !="); }
};

template <class D>
class InstanceCounter {
public:
    static long created() { throw std::logic_error("TODO: created"); }
    static long alive() { throw std::logic_error("TODO: alive"); }
    // TODO: count constructions and destructions
};

template <class Base, class Derived>
struct Cloneable : Base {
    std::unique_ptr<Base> clone() const override { throw std::logic_error("TODO: clone"); }
};

template <class D>
class Printable {
public:
    std::string to_string() const { throw std::logic_error("TODO: to_string"); }
};

}  // namespace idioms
