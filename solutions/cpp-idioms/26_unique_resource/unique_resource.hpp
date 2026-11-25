// Reference solution for problem 26.
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
    UniqueResource(R r, D d) : resource_(std::move(r)), deleter_(std::move(d)), execute_on_reset_(true) {}
    UniqueResource(const UniqueResource&) = delete;
    UniqueResource& operator=(const UniqueResource&) = delete;
    UniqueResource(UniqueResource&& o) noexcept(std::is_nothrow_move_constructible_v<R> &&
                                                std::is_nothrow_move_constructible_v<D>)
        : resource_(std::move(o.resource_)),
          deleter_(std::move(o.deleter_)),
          execute_on_reset_(std::exchange(o.execute_on_reset_, false)) {}
    UniqueResource& operator=(UniqueResource&& o) noexcept(std::is_nothrow_move_assignable_v<R> &&
                                                           std::is_nothrow_move_assignable_v<D>) {
        if (this != &o) {
            reset();
            resource_ = std::move(o.resource_);
            deleter_ = std::move(o.deleter_);
            execute_on_reset_ = std::exchange(o.execute_on_reset_, false);
        }
        return *this;
    }
    ~UniqueResource() { reset(); }

    void reset() {
        if (execute_on_reset_) {
            execute_on_reset_ = false;
            deleter_(resource_);
        }
    }
    void reset(R r) {
        reset();
        resource_ = std::move(r);
        execute_on_reset_ = true;
    }
    void release() noexcept { execute_on_reset_ = false; }
    const R& get() const noexcept { return resource_; }
    const D& get_deleter() const noexcept { return deleter_; }
    bool valid() const noexcept { return execute_on_reset_; }

private:
    R resource_{};
    [[no_unique_address]] D deleter_{};
    bool execute_on_reset_ = false;
};

template <class R, class D, class Invalid>
UniqueResource<std::decay_t<R>, std::decay_t<D>> make_unique_resource_checked(R&& r, const Invalid& invalid, D&& d) {
    const bool ok = !(r == invalid);
    UniqueResource<std::decay_t<R>, std::decay_t<D>> res(std::forward<R>(r), std::forward<D>(d));
    if (!ok) res.release();
    return res;
}

struct CloseFd {
    void operator()(int fd) const noexcept { ::close(fd); }
};
using Fd = UniqueResource<int, CloseFd>;

inline std::pair<Fd, Fd> open_pipe() {
    int fds[2];
    if (::pipe(fds) != 0) throw std::system_error(errno, std::generic_category(), "pipe");
    return {Fd(fds[0], CloseFd{}), Fd(fds[1], CloseFd{})};
}

}  // namespace idioms
