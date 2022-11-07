#include <functional>
#include <iostream>
#include <memory>

template <typename T>
struct CallableBase;

template <typename R_, typename... Args_>
struct CallableBase<R_(Args_...)> {
    CallableBase()          = default;
    virtual ~CallableBase() = default;

    virtual R_ invoke(Args_... args) = 0;
};
template <typename T, typename Functor>
struct Callable;

template <typename R_, typename... Args_, typename Functor_>
struct Callable<R_(Args_...), Functor_> : CallableBase<R_(Args_...)> {
    Callable(Functor_ t) : f_{std::move(t)} {}

    R_ invoke(Args_... args) override {
        return std::invoke(f_, std::forward<Args_>(args)...);
    }
    Functor_ f_;
};

template <typename T>
struct Function;

template <typename R_, typename... Args_>
struct Function<R_(Args_...)> {

    template <typename T>
    Function(T &&t) {
        f_.reset(new Callable<R_(Args_...), T>(std::forward<T>(t)));
    }

    Function(Function const &f) {}

    template <typename... Args>
    R_ operator()(Args &&...args) {
        return f_->invoke(std::forward<Args>(args)...);
    }

    std::unique_ptr<CallableBase<R_(Args_...)>> f_;
};

struct A {
    A() = default;
    A(const A &a) { std::cerr << "copy" << std::endl; }
    A(A &a) { std::cerr << "copy" << std::endl; }
};

int main() {
    Function<int(A)> f = [](const A &x) -> int {
        std::cerr << "call it" << std::endl;
        return 5;
    };
    std::function<int(A &)> ff = [](A &x) -> int {
        std::cerr << "call it" << std::endl;
        return 5;
    };
    /* std::function<int(int)> ff; */
    A    wd;
    auto sb = [](A x) -> int {
        std::cerr << "call it" << std::endl;
        return 5;
    };
    f(wd);
    ff(wd);
    sb(wd);
}
