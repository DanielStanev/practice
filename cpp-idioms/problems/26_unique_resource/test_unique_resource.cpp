#include "unique_resource.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
std::vector<int> g_released;
struct Recorder {
    void operator()(int h) const { g_released.push_back(h); }
};
struct EmptyDeleter {
    void operator()(int) const noexcept {}
};
bool fd_is_open(int fd) { return ::fcntl(fd, F_GETFD) != -1; }
}  // namespace

TEST(releases_once_on_destruction) {
    g_released.clear();
    {
        UniqueResource<int, Recorder> r(7, Recorder{});
        CHECK(r.valid());
        CHECK_EQ(r.get(), 7);
    }
    CHECK(g_released == (std::vector<int>{7}));
}

TEST(zero_is_a_valid_handle) {
    g_released.clear();
    { UniqueResource<int, Recorder> r(0, Recorder{}); }
    CHECK(g_released == (std::vector<int>{0}));
}

TEST(default_is_disengaged) {
    g_released.clear();
    { UniqueResource<int, Recorder> r; CHECK_FALSE(r.valid()); }
    CHECK(g_released.empty());
}

TEST(move_transfers_ownership) {
    g_released.clear();
    {
        UniqueResource<int, Recorder> a(1, Recorder{});
        UniqueResource<int, Recorder> b(std::move(a));
        CHECK_FALSE(a.valid());
        CHECK(b.valid());
        UniqueResource<int, Recorder> c(2, Recorder{});
        c = std::move(b);  // releases 2 first
        CHECK(g_released == (std::vector<int>{2}));
        CHECK_EQ(c.get(), 1);
        auto& alias = c;
        c = std::move(alias);  // self-move: harmless
        CHECK(c.valid());
    }
    CHECK(g_released == (std::vector<int>{2, 1}));
    CHECK_TRAIT(!std::is_copy_constructible_v<UniqueResource<int, Recorder>>);
    CHECK_TRAIT(std::is_nothrow_move_constructible_v<UniqueResource<int, Recorder>>);
}

TEST(reset_and_release) {
    g_released.clear();
    UniqueResource<int, Recorder> r(3, Recorder{});
    r.reset(4);
    CHECK(g_released == (std::vector<int>{3}));
    r.release();
    CHECK_FALSE(r.valid());
    r.reset();  // nothing to do
    CHECK(g_released == (std::vector<int>{3}));
    r.reset(5);
    r.reset();
    r.reset();
    CHECK(g_released == (std::vector<int>{3, 5}));
}

TEST(checked_factory) {
    g_released.clear();
    {
        auto bad = make_unique_resource_checked(-1, -1, Recorder{});
        auto good = make_unique_resource_checked(9, -1, Recorder{});
        CHECK_FALSE(bad.valid());
        CHECK(good.valid());
    }
    CHECK(g_released == (std::vector<int>{9}));
}

TEST(empty_deleter_takes_no_space) {
    // With a 1-byte handle, padding cannot hide the deleter: {char, bool} is 2
    // bytes, {char, EmptyDeleter, bool} is 3 unless the deleter takes no space.
    CHECK_EQ(sizeof(UniqueResource<char, EmptyDeleter>), std::size_t{2});
}

TEST(real_file_descriptors) {
    int r = -1, w = -1;
    {
        auto [rd, wr] = open_pipe();
        r = rd.get();
        w = wr.get();
        CHECK(fd_is_open(r));
        CHECK(fd_is_open(w));
        const char msg[] = "ping";
        CHECK_EQ(::write(wr.get(), msg, 4), 4L);
        char buf[8] = {};
        CHECK_EQ(::read(rd.get(), buf, 4), 4L);
        CHECK_EQ(std::strcmp(buf, "ping"), 0);
        Fd moved = std::move(wr);
        CHECK(fd_is_open(w));
    }
    CHECK_FALSE(fd_is_open(r));
    CHECK_FALSE(fd_is_open(w));
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
