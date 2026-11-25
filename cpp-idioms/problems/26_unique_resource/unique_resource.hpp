// Problem 26: UniqueResource.
// Read README.md first. Implement everything marked TODO.
#pragma once

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <utility>

#include <unistd.h>

namespace idioms {

template <class R, class D>
class UniqueResource {
public:
    UniqueResource() = default;
    UniqueResource(R r, D d) {
        (void)r;
        (void)d;
        throw std::logic_error("TODO: UniqueResource(R, D)");
    }
    // TODO: move constructor, move assignment, no copies, destructor.

    void reset() { throw std::logic_error("TODO: reset()"); }
    void reset(R r) {
        (void)r;
        throw std::logic_error("TODO: reset(R)");
    }
    void release() noexcept {}
    const R& get() const noexcept { return resource_; }
    const D& get_deleter() const noexcept { return deleter_; }
    bool valid() const noexcept { return false; }  // TODO

private:
    R resource_{};
    D deleter_{};
    bool execute_on_reset_ = false;  // do we own something? TODO: use it
};

template <class R, class D, class Invalid>
UniqueResource<std::decay_t<R>, std::decay_t<D>> make_unique_resource_checked(R&& r, const Invalid& invalid, D&& d) {
    (void)r;
    (void)invalid;
    (void)d;
    throw std::logic_error("TODO: make_unique_resource_checked");
}

struct CloseFd {
    void operator()(int fd) const noexcept { ::close(fd); }
};
using Fd = UniqueResource<int, CloseFd>;

inline std::pair<Fd, Fd> open_pipe() { throw std::logic_error("TODO: open_pipe"); }

}  // namespace idioms
