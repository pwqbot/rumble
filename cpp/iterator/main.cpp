#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <tuple>
#include <vector>

template <typename T>
struct MyIter {
    explicit MyIter(T *val) : current_{val} {}

    auto operator*() -> typename T::value_type { return current_->x_; }
    auto operator*() const -> typename T::value_type { return current_->x_; }

    auto operator++() -> MyIter & {
        if (current_ != nullptr) {
            current_ = current_->next.get();
        }
        return *this;
    }

    auto operator++(int) -> MyIter {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    auto operator==(const MyIter &b) -> bool { return current_ == b.current_; }
    using iterator_category = std::forward_iterator_tag;
    using value_type        = T;
    using difference_type   = int;
    using pointer           = T *;
    using reference         = T &;

    T *current_{};
    T *next_{};
};

template <typename Val>
struct Mylist {
    Mylist() = default;

    struct Node {
        explicit Node(Val v, std::unique_ptr<Node> next)
            : x_{v}, next{std::move(next)} {}
        auto operator<(const Node &b) -> bool { return x_ < b.x_; }
        Val  x_;
        std::unique_ptr<Node> next;
        using value_type = Val;
    };

    template <typename... Args>
    auto emplace(Args &&...args) {
        if (size_ != 0) {
            std::unique_ptr<Node> new_head =
                std::make_unique<Node>(args..., nullptr);
            new_head->next = std::move(head_);
            head_          = std::move(new_head);
        } else {
            head_ = std::make_unique<Node>(args..., nullptr);
        }
        size_++;
    }

    auto begin() -> MyIter<Node> { return MyIter<Node>(head_.get()); }
    auto end() -> MyIter<Node> { return MyIter<Node>(nullptr); }

    std::unique_ptr<Node> head_{};
    size_t                size_{0};
};

auto main() -> int {
    std::vector<int>          v;
    std::list<int>            l;
    std::iterator_traits<int> c;
    v.begin();
    Mylist<int> list;
    list.emplace(5);
    list.emplace(6);
    list.emplace(7);
    auto sb = list.begin();
    std::cerr << sb.current_->x_ << std::endl;
    sb++;
    std::cerr << sb.current_->x_ << std::endl;
    std::cerr << std::distance(list.begin(), list.end());
    /* std::cerr << std::count(list.begin(), list.end(), 5); */
    /* std::sort(list.begin(), list.end()); */
}
