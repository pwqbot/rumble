#include <list>
#include <ranges>

void sort_list() {
    std::list<int> list{3, 2, 1};
    std::ranges::sort(list.begin(), list.end());
}
