#include <array>
#include <iostream>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

template <class T, std::size_t N>
class static_vector {
    // NOTE: aligned alloc memory in stack, without init T
    // https://stackoverflow.com/questions/50271304/what-is-the-purpose-of-stdaligned-storage
    std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N> data;
    std::size_t                                                  m_size = 0;

  public:
    template <typename... Args>
    auto emplace_back(Args &&...args) {
        if (m_size >= 0) {
        }

        new (&data[m_size]) T(std::forward<Args>(args)...);
    }

    ~static_vector() {
        for (std::size_t pos = 0; pos < m_size; ++pos) {
            std::destroy_at(std::launder(reinterpret_cast<T *>(&data[pos])));
        }
    }
};

template <typename T>
struct op {
    std::aligned_storage_t<sizeof(T), alignof(T)> buf_;
    bool                                          engaged_;

    op() : engaged_(false) {}
    op(const T &t) : engaged_(true) { ::new (&buf_) T(t); }
    // NOTE: op is trivial destructible is T is trivially destructible
    ~op() {
        if (engaged_) {
            reinterpret_cast<T &>(buf_).~T();
        }
    }
};

struct sb {
    ~sb();
};

namespace trivial_op {

struct op {};

} // namespace trivial_op
//

namespace ass {
struct Data {
    Data() noexcept { puts("default ctor"); }
    Data(const Data &) noexcept { puts("copy ctor"); }
    Data(Data &&) noexcept { puts("move ctor"); }
    Data &operator=(Data &&) noexcept {
        puts("move assign");
        return *this;
    }
    // Data &operator=(const Data &) noexcept {
    //     puts("copy assign");
    //     return *this;
    // }
    ~Data() { puts("dtor"); }
};

auto v() {
    std::vector<Data> r;
    r.emplace_back();
    return r;
}

int main() {
    Data d;
    d = v()[0]; // What kind of assignment is this?It’s a copy assignment.
}
} // namespace ass

auto main() -> int {
    int *p = new int(10);
    int *q = new (p) int(5);

    static_vector<std::string, 10> v1;
    v1.emplace_back(5, '*');
    std::cout << *p << std::endl;
    auto wd = op(5);
    static_assert(std::is_trivially_destructible_v<sb> == true);
}
