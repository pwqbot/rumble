#include <catch2/catch_all.hpp>
#include <cctype>
#include <range/v3/action.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/drop_while.hpp>
#include <range/v3/view/ref.hpp>
#include <range/v3/view/repeat.hpp>
#include <range/v3/view/trim.hpp>
#include <ranges>

TEST_CASE("view", "[view]") {
    std::string s = "a";
    std::string b = "qyy";
    auto        zips =
        ranges::views::zip(s | ranges::views::cycle, b) | ranges::to_vector;
    REQUIRE(zips == std::vector<std::pair<char, char>>{
                        {'a', 'q'}, {'a', 'y'}, {'a', 'y'}});
}

TEST_CASE("modify view") {
    std::string s = "abc";

    ranges::for_each(s | ranges::views::all, [](auto &c) { c = 'x'; });
    REQUIRE(s == "xxx");
}

TEST_CASE("Trim Str") {
    auto trim_front = ranges::views::drop_while(::isspace);
    auto trim_back =
        ranges::views::reverse | trim_front | ranges::views::reverse;
    auto trim_str = trim_front | trim_back;

    auto fuz_s  = std::string("\nasdfasdfaas   \t");
    auto trim_s = ranges::views::all(fuz_s) | trim_str | ranges::to<std::string>();
    REQUIRE(trim_s == "asdfasdfaas");
}
