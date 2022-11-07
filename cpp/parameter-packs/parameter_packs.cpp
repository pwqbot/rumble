#include <assert.h>
#include <functional>
#include <iostream>
#include <ostream>
#include <set>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

template <typename... T>
void call(T... t) {
    auto lam = [... t = std::forward<T>(t)]() {

    };
    auto with_copy = [t...] {
        call(t...);
    };
}

namespace idioms {

// ================= recursing over argument lists ============================
void printall() {}
void printall(const auto &first, const auto &...rest) {
    std::cout << first;
    printall(rest...);
}
void printall2(const auto &...args) {
    (std::cout << ... << args);
}

// ================== recursing over template parameters =======================
template <char... Cs>
struct string_holder {
    static constexpr char   value[] = {Cs..., '\0'};
    static constexpr size_t len     = sizeof...(Cs);
    constexpr explicit      operator const char *() const { return value; }
};

template <size_t N, char... Cs>
consteval auto index_string() {
    if constexpr (N < 10) {
        return string_holder<N + '0', Cs...>{};
    } else {
        return index_string<N / 10, N % 10 + '0', Cs...>();
    }
}

constinit const char *const ten =
    static_cast<const char *>(index_string<123>());

template <char... Cs>
consteval auto add_comma(string_holder<> /*unused*/, string_holder<Cs...> out) {
    return out;
}

template <char In0, char... InRest, char... Out>
consteval auto add_comma(string_holder<In0, InRest...> /*unused*/,
                         string_holder<Out...> /*unused*/ = {}) {
    if constexpr (sizeof...(InRest) % 3 == 0 && sizeof...(InRest) > 0) {
        return add_comma(string_holder<InRest...>{},
                         string_holder<Out..., In0, ','>{});
    } else {
        return add_comma(string_holder<InRest...>{},
                         string_holder<Out..., In0>{});
    }
}

constinit const char *const comma_ten =
    static_cast<const char *>(add_comma(index_string<1000000>()));

// =============================== fold expression =============================
template <typename T, typename... E>
void multi_insert(T &t, E &&...e) {
    (t.insert(std::forward<E>(e)), ...);
}

template <typename T, typename F>
auto tuple_find(const T &t, F &&f) -> std::size_t {
    return std::apply(
        [&f](const auto &...e) {
            int r = 0;
            (((std::forward<F>(f)(e)) || (++r, false)) || ...);
            return r;
        },
        t);
}

// ============================== lambda overload ==============================
template <typename... L>
struct multilambda : L... {
    using L::operator()...;
    constexpr explicit multilambda(L... lam) : L(std::move(lam))... {}
};

template <typename... Args>
void test_multilambda(Args &&...args) {
    multilambda action{
        [](int i) { std::cout << "int" << std::endl; },
        [](double i) { std::cout << "double" << std::endl; },
        [](char i) { std::cout << "char" << std::endl; },
    };
    (action(args), ...);
}

// ======================== unevaluated lambda =================================
template <typename T>
using tuple_ptrs =
    decltype(std::apply([](auto... t) { return std::tuple(&t...); }),
             std::declval<T>());

// ============================ capute pack from tuple =========================
auto tuple_mult(auto scalar, auto tpl) {
    return std::apply([&scalar]<typename... T>(
                          T... t) { return std::tuple(T(scalar * t)...); },
                      tpl);
}

template <typename T>
auto tuple_add(const T &ta, const T &tb) {
    return [&ta, &tb ]<std::size_t... I>(std::index_sequence<I...>) {
        return std::tuple(std::get<I>(ta) + std::get<I>(tb)...);
    }
    (std::make_index_sequence<std::tuple_size_v<T>>());
}

// =================== recursive types through inheritance =====================
template <typename... T>
struct HList;

template <std::size_t N, typename HL>
auto get(HL &&hl) -> decltype(auto) {
    return drop<N>(std::forward<HL>(hl)).head();
}

template <>
struct HList<> {
    static constexpr std::size_t len = 0;
};

template <typename... T>
HList(T...) -> HList<T...>;

template <typename T0, typename... TRest>
struct HList<T0, TRest...> : HList<TRest...> {
    using head_type = T0;
    using tail_type = HList<TRest...>;

    static constexpr std::size_t    len = sizeof...(TRest) + 1;
    [[no_unique_address]] head_type value_;

    template <typename U0, typename... URest>
    explicit HList(U0 &&u0, URest &&...urest)
        : value_{std::forward<U0>(u0)},
          tail_type(std::forward<URest>(urest)...) {}

    template <typename... U>
    auto emplace_back(U &&...u) {
        return std::apply(
            [this, &u...](auto... v) {
                return HList<T0, TRest..., U...>(v..., std::forward<U>(u)...);
            },
            [this]<std::size_t... I>(std::index_sequence<I...>) {
                return std::tuple(get<I>(*this)...);
            }(std::make_index_sequence<len>{}));
    }

    auto               head()               &-> head_type               &{ return value_; }
    [[nodiscard]] auto head() const & -> const head_type & { return value_; }
    auto               head()               &&-> head_type               &&{ return value_; }

    auto               tail()               &-> tail_type               &{ return *this; }
    [[nodiscard]] auto tail() const & -> const tail_type & { return *this; }
    auto               tail()               &&-> tail_type               &&{ return *this; }
};

template <std::size_t N, typename HL>
auto drop(HL &&hl) -> decltype(auto) {
    if constexpr (N) {
        return drop<N - 1>(std::forward<HL>(hl).tail());
    } else {
        return std::forward<HL>(hl);
    }
}

template <typename F, typename HL>
auto map(F &&f, HL &&hl) -> decltype(auto) {
    [&f, &hl ]<std::size_t... I>(std::index_sequence<I...>)->decltype(auto) {
        return HList(std::forward<F>(f)(get<I>(hl))...);
    }
    (std::make_index_sequence<std::remove_cvref_t<HL>::len>{});
}

// ================ runtime constant -> compile time constant ==================
template <std::size_t I, typename R, typename F>
auto with_integral_const(F f) -> R {
    return f(std::integral_constant<std::size_t, I>{});
}

template <size_t N, typename R = void, typename F>
constexpr auto with_n(int n, F &&f) -> R {
    constexpr auto index_array =
        []<std::size_t... I>(std::index_sequence<I...>) {
        return std::array{with_integral_const<I, R, F &&>...};
    }
    (std::make_index_sequence<N>{});
    return index_array.at(n)(std::forward<F>(f));
}

template <typename T, typename... Args>
void set_index(T &t, std::size_t n, Args &&...args) {
    return with_n<std::variant_size_v<T>>(n, [&t, &args...](auto i) {
        t.template emplace<i>(std::forward<Args>(args)...);
    });
}

} // namespace idioms

void test_multilambda() {
    std::cout << "test_multilambda" << std::endl;
    idioms::test_multilambda(1, 2., 'c');
}

void test_HList() {
    std::cout << "test_HList" << std::endl;
    auto  list = idioms::HList(0, 1, 2);
    auto &sb   = list.tail();
    auto  x    = idioms::get<1>(list);
    std::cout << x << std::endl;
    auto list2 = list.emplace_back(3, 4);
    auto y     = idioms::get<3>(list2);
    std::cout << y << std::endl;
}

void test_set_index() {
    std::cout << "test_set_index" << std::endl;
    auto wd = std::variant<int,double>(1);
    idioms::set_index(wd, 0, 2);
    std::cout << std::get<0>(wd) << std::endl;
}

auto main() -> int {
    // dd(fun);
    std::cout << idioms::ten << std::endl;
    std::cout << idioms::comma_ten << std::endl;
    std::set<int> s;
    ::idioms::multi_insert(s, 1, 2, 3);
    std::tuple t(-2, -1, 0U, 1UL, 2ULL);
    assert(::idioms::tuple_find(
               t, [](auto i) { return std::cmp_greater(i, -0); }) == 2);
    std::tuple();
    test_HList();
    test_multilambda();
    test_set_index();
    // std::bind();
}
