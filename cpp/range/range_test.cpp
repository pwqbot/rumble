#include <catch2/catch_all.hpp>
#include <range/v3/action.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/repeat.hpp>

TEST_CASE("view", "[view]") {
    std::string s = "a";
    std::string b = "yy";
    auto        zips =
        ranges::views::zip(s | ranges::views::cycle, b) | ranges::to_vector;
    REQUIRE(zips == std::vector<std::pair<char, char>>{
                        {'a', 'q'}, {'a', 'y'}, {'a', 'y'}});
}
