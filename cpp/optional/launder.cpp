#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

namespace pb1 {
struct X {
    const int n;
};

union U {
    X     x;
    float f;
};

void tong() {
    U u  = {{1}};
    u.f  = 5.f;
    X *p = new (&u.x) X{2};

    // refer to new name, which is ok
    assert(p->n == 2);

    // BUG: refer to previous name, which has const member and may be trigger
    // BUG: optimization
    assert(u.x.n == 2);
}
} // namespace pb1

namespace pb2 {
template <typename T>
class coreoptional {
  private:
    T payload;

  public:
    explicit coreoptional(const T &t) : payload(t) {}
    template <typename... Args>
    auto emplace(Args &&...args) {
        payload.~T();
        ::new (&payload)
            T(std::forward<Args>(args)...); // NOTE(1): * not hold return value
    }
    auto operator*() const & -> const T & { return payload; }
};

template <typename T>
class coreoptionalFix {
  private:
    T  payload;
    T *p; // NOTE: introduce new name to avoid optimization

  public:
    explicit coreoptionalFix(const T &t) : payload(t), p(&payload) {}
    template <typename... Args>
    auto emplace(Args &&...args) {
        payload.~T();
        p = ::new (&payload) T(std::forward<Args>(
            args)...); // NOTE: introduce new name to avoid optimization
    }
    auto operator*() const & -> const T & { return *p; }
};

struct Y {
    const int _i;
    Y(int i) : _i(i) {}
};

void test() {
    coreoptional<Y> optStr{42};
    optStr.emplace(2);
    assert((*optStr)._i == 2); // BUG: undefined behavior

    coreoptionalFix<Y> optStr2{52};
    optStr2.emplace(2);
    assert((*optStr2)._i == 2); // OK
}
} // namespace pb2

namespace pb3 {
struct A {
    virtual int f();
};

struct B : A {
    virtual int f() {
        new (this) A;
        return 1;
    }
};

int A::f() {
    new (this) B;
    return 2;
}

int h() {
    A a;
    // BUG: vptr is a implicitly const member of A, which may trigger
    // BUG: devitualization optimization
    int n = a.f();
    int m = std::launder(&a)->f();
    return n + m; // NOTE: right answer is 3.
}
} // namespace pb3
//

namespace fix {
struct X {
    const int n;
};

void use_launder() {
    X *p = new X{7};
    new (p) X{42};
    int b = p->n;               // BUG: undefined behavior
    int c = std::launder(p)->n; // ok
    int d = p->n;               // BUG: undefined behavior
}

} // namespace fix
