#include "visitors.hpp"

#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "testing.hpp"

using namespace idioms;

namespace {
template <class... Fs>
concept CanBuildOverloaded = requires(Fs... fs) { overloaded{fs...}; };

template <class Dummy>
void overloaded_impl() {
    auto a = [](int) { return 1; };
    auto b = [](const std::string&) { return 2; };
    auto c = [](const auto&) { return 3; };
    if constexpr (CanBuildOverloaded<decltype(a), decltype(b), decltype(c)> &&
                  std::is_invocable_v<overloaded<decltype(a), decltype(b), decltype(c)>, int>) {
        overloaded o{a, b, c};
        CHECK_EQ(o(5), 1);
        CHECK_EQ(o(std::string("x")), 2);
        CHECK_EQ(o(2.5), 3);  // the generic one catches the rest
        std::variant<int, std::string, double> v = 1.0;
        CHECK_EQ(std::visit(o, v), 3);
        v = std::string("s");
        CHECK_EQ(std::visit(o, v), 2);
    } else {
        ::testing::fail(__FILE__, __LINE__, "overloaded is not usable yet");
    }
}
}  // namespace

TEST(overloaded_combines_lambdas) { overloaded_impl<void>(); }

TEST(describe_messages) {
    CHECK_EQ(describe(Add{1, 5, 100.5}), std::string("add 5@100.5"));
    CHECK_EQ(describe(Cancel{7}), std::string("cancel 7"));
    CHECK_EQ(describe(Execute{3, 10}), std::string("exec 3x10"));
}

TEST(apply_to_book) {
    Book b;
    apply_message(b, Add{1, 100, 10.0});
    apply_message(b, Add{2, 50, 10.5});
    apply_message(b, Execute{1, 30});
    CHECK_EQ(b.at(1), 70L);
    apply_message(b, Execute{2, 50});
    CHECK_EQ(b.count(2), std::size_t{0});
    apply_message(b, Cancel{1});
    CHECK(b.empty());
    apply_message(b, Execute{99, 1});  // unknown id: ignored
    CHECK(b.empty());
}

TEST(count_by_type_values) {
    std::vector<Msg> msgs{Add{1, 1, 1.0}, Cancel{1}, Add{2, 1, 1.0}, Execute{2, 1}, Add{3, 1, 1.0}};
    auto c = count_by_type(msgs);
    CHECK_EQ(c[0], std::size_t{3});
    CHECK_EQ(c[1], std::size_t{1});
    CHECK_EQ(c[2], std::size_t{1});
}

TEST(tree_sum_recursive) {
    Node leaf{5L};
    CHECK_EQ(tree_sum(leaf), 5L);
    Node tree{std::vector<Node>{Node{1L}, Node{std::vector<Node>{Node{2L}, Node{3L}}}, Node{std::vector<Node>{}},
                                Node{4L}}};
    CHECK_EQ(tree_sum(tree), 10L);
}

int main(int argc, char** argv) { return testing::run_all(argc, argv); }
