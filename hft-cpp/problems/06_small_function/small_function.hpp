// Problem 06: SmallFunction, a move-only std::function with small-buffer optimisation
// Read README.md first. Implement everything marked TODO.
// You may add private members and helpers, but keep the public interface.
// Do not use std::function or std::any inside.
#pragma once

#include <cstddef>
#include <functional>  // std::bad_function_call, std::invoke
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hft {

template <typename Signature, std::size_t BufferSize = 32>
class SmallFunction;

template <typename R, typename... Args, std::size_t BufferSize>
class SmallFunction<R(Args...), BufferSize> {
public:
    SmallFunction() = default;
    SmallFunction(std::nullptr_t) {}

    // Stores any callable invocable as R(Args...). Callables that fit in the
    // buffer (size, alignment, noexcept move) are stored inline, the rest on
    // the heap.
    // TODO: constrain: F must be invocable as R(Args...) and must not be
    // SmallFunction itself (or this hijacks the move constructor).
    template <typename F>
    SmallFunction(F&& f) {
        (void)f;
        throw std::logic_error("TODO: SmallFunction(F&&)");
    }

    // TODO: copying is not supported.

    SmallFunction(SmallFunction&& o) {
        (void)o;
        throw std::logic_error("TODO: move constructor");
    }

    SmallFunction& operator=(SmallFunction&& o) {
        (void)o;
        throw std::logic_error("TODO: move assignment");
    }

    SmallFunction& operator=(std::nullptr_t) {
        throw std::logic_error("TODO: operator=(nullptr)");
    }

    ~SmallFunction() {
        // TODO: destroy the stored callable, if any.
    }

    // Destroys the stored callable; the function becomes empty.
    void reset() { throw std::logic_error("TODO: reset"); }

    // Calls the stored callable. Throws std::bad_function_call when empty.
    R operator()(Args... args) {
        ((void)args, ...);
        throw std::logic_error("TODO: operator()");
    }

    explicit operator bool() const { return false; /* TODO */ }

    // True if the callable lives in the internal buffer.
    bool is_inline() const { return false; /* TODO */ }

private:
    alignas(std::max_align_t) std::byte buf_[BufferSize];
    // TODO: whatever you need to remember how to call, move and destroy the
    // stored callable. Keep it to at most two pointers.
};

}  // namespace hft
