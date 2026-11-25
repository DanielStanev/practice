#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace hft {

template <typename Signature, std::size_t BufferSize = 32>
class SmallFunction;

template <typename R, typename... Args, std::size_t BufferSize>
class SmallFunction<R(Args...), BufferSize> {
    template <typename F>
    static constexpr bool fits_inline = sizeof(F) <= BufferSize &&
                                        alignof(F) <= alignof(std::max_align_t) &&
                                        std::is_nothrow_move_constructible_v<F>;

    struct VTable {
        R (*invoke)(void* storage, Args&&... args);
        void (*move)(void* dst, void* src) noexcept;  // move-construct dst from src, destroy src
        void (*destroy)(void* storage) noexcept;
        bool is_inline;
    };

    template <typename F>
    static F* inline_ptr(void* s) noexcept {
        return std::launder(reinterpret_cast<F*>(s));
    }
    template <typename F>
    static F*& heap_ptr(void* s) noexcept {
        return *std::launder(reinterpret_cast<F**>(s));
    }

    template <typename F>
    static constexpr VTable inline_vtable{
        [](void* s, Args&&... args) -> R {
            return std::invoke(*inline_ptr<F>(s), std::forward<Args>(args)...);
        },
        [](void* dst, void* src) noexcept {
            ::new (dst) F(std::move(*inline_ptr<F>(src)));
            std::destroy_at(inline_ptr<F>(src));
        },
        [](void* s) noexcept { std::destroy_at(inline_ptr<F>(s)); },
        true,
    };

    template <typename F>
    static constexpr VTable heap_vtable{
        [](void* s, Args&&... args) -> R {
            return std::invoke(*heap_ptr<F>(s), std::forward<Args>(args)...);
        },
        [](void* dst, void* src) noexcept { ::new (dst) F*(heap_ptr<F>(src)); },
        [](void* s) noexcept { delete heap_ptr<F>(s); },
        false,
    };

public:
    SmallFunction() noexcept = default;
    SmallFunction(std::nullptr_t) noexcept {}

    template <typename F>
        requires(!std::is_same_v<std::remove_cvref_t<F>, SmallFunction> &&
                 std::is_invocable_r_v<R, std::decay_t<F>&, Args...>)
    SmallFunction(F&& f) {
        using D = std::decay_t<F>;
        if constexpr (fits_inline<D>) {
            ::new (static_cast<void*>(buf_)) D(std::forward<F>(f));
            vt_ = &inline_vtable<D>;
        } else {
            ::new (static_cast<void*>(buf_)) D*(new D(std::forward<F>(f)));
            vt_ = &heap_vtable<D>;
        }
    }

    SmallFunction(const SmallFunction&) = delete;
    SmallFunction& operator=(const SmallFunction&) = delete;

    SmallFunction(SmallFunction&& o) noexcept {
        if (o.vt_) {
            o.vt_->move(buf_, o.buf_);
            vt_ = std::exchange(o.vt_, nullptr);
        }
    }

    SmallFunction& operator=(SmallFunction&& o) noexcept {
        if (this != &o) {
            reset();
            if (o.vt_) {
                o.vt_->move(buf_, o.buf_);
                vt_ = std::exchange(o.vt_, nullptr);
            }
        }
        return *this;
    }

    SmallFunction& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    ~SmallFunction() { reset(); }

    void reset() noexcept {
        if (vt_) {
            vt_->destroy(buf_);
            vt_ = nullptr;
        }
    }

    R operator()(Args... args) {
        if (!vt_) throw std::bad_function_call();
        return vt_->invoke(buf_, std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept { return vt_ != nullptr; }
    bool is_inline() const noexcept { return vt_ && vt_->is_inline; }

private:
    alignas(std::max_align_t) std::byte buf_[BufferSize < sizeof(void*) ? sizeof(void*) : BufferSize];
    const VTable* vt_ = nullptr;
};

}  // namespace hft
