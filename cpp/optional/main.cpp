#include <array>
#include <iostream>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// this is a vector in stack
template <class T, std::size_t N>
class static_vector {
    // NOTE: aligned alloc memory in stack, without init T
    // https://stackoverflow.com/questions/50271304/what-is-the-purpose-of-stdaligned-storage
    std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N> data_;
    std::size_t                                                  m_size_ = 0;

  public:
    template <typename... Args>
    auto emplace_back(Args &&...args) -> void {
        if (m_size_ >= N - 1) {
            throw "fuck";
        }
        new (&data_[m_size_++]) T(std::forward<Args>(args)...);
    }
    ~static_vector() {
        for (std::size_t pos = 0; pos < m_size_; ++pos) {
            // NOTE: see std::launder in launder.cpp
            std::destroy_at(std::launder(reinterpret_cast<T *>(&data_[pos])));
        }
    }
};

namespace trivial_op {

template <typename T>
struct op {
    T    val_; // BUG: we don't want T's default constructor to call
    bool engaged_;
};

} // namespace trivial_op

namespace smarter_op {

template <typename T>
struct op {
    std::aligned_storage_t<sizeof(T), alignof(T)> buf_;
    bool                                          engaged_;
    op() : engaged_(false) {}
    op(const T &t) : engaged_(true) { ::new ((void *)&buf_) T(t); }
    // NOTE: op is trivial destructible if T is trivially destructible
    ~op() {
        if (engaged_) {
            reinterpret_cast<T &>(buf_).~T();
        }
    }
};

template <typename T>
struct union_op {
    union {
        char dummy_;
        T    val_;
    };
    bool engaged_;
    union_op() : dummy_{0}, engaged_(false) {}
    union_op(const T &t) : engaged_(true) {}
    // NOTE: op is trivial destructible if T is trivially destructible
    ~union_op() {
        if (engaged_) {
            val_.~T();
        }
    }
};

} // namespace smarter_op

namespace qwqop {
namespace detail {

template <typename T, typename E = void>
struct op_storage {
    union {
        char dummy_;
        T    val_;
    };
    bool engaged_;

    op_storage() : dummy_{0}, engaged_(false) {}
    op_storage(const T &t) : engaged_(true) {}
    // NOTE: op is trivial destructible if T is trivially destructible
    ~op_storage() {
        if (engaged_) {
            val_.~T();
        }
    }
};

template <typename T>
struct op_storage<T, std::enable_if_t<std::is_trivially_destructible_v<T>>> {
    union {
        char dummy_;
        T    val_;
    };
    bool engaged_;

    op_storage() : dummy_{0}, engaged_(false) {}
    op_storage(const T &t) : engaged_(true) {}
    // NOTE: op is trivial destructible if T is trivially destructible
    ~op_storage() = default;
};

} // namespace detail

template <typename T>
struct optional {
    detail::op_storage<T> v_;
    ~optional() = default;

    auto check_engaged() -> void {
        if (!v_.engaged_) {
            throw "fuck up";
        }
    }
    auto value() && -> T && {
        check_engaged();
        return std::move(v_.val_);
    }
    auto value() const && -> const T && {
        check_engaged();
        return std::move(v_);
    }
    auto value() const & -> const T & {
        check_engaged();
        return v_;
    }
    auto value() & -> T & {
        check_engaged();
        return v_;
    }
};

} // namespace qwqop

auto main() -> int {
    auto *p = new char(10);
    int  *q = new (p) int(5);

    static_vector<std::string, 10> v1;
    v1.emplace_back(5, '*');

    std::cout << *p << std::endl;
    struct A {};
    auto wd = qwqop::optional<A>();
    static_assert(std::is_trivially_destructible_v<decltype(wd)> ==
                  std::is_trivially_destructible_v<A>);
}
