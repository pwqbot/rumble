#pragma once
#include <type_traits>

template <template <typename... T> class Tp, typename T>
struct BindTp {
    template <typename... Args>
    using type = Tp<T, Args...>;
};

// because template alias cannot fucking deduction
#define should_compile(lambda)                                                 \
    typename BindTp<::constraint::detail::compiles,                            \
                    decltype(lambda)>::template type

#define should_compile_t(lambda, T)                                            \
    typename BindTp<::constraint::detail::compiles,                            \
                    decltype(lambda)>::template type<T>

namespace constraint {

namespace detail {

namespace compiles_detail {

template <typename Lambda, typename vvv = void, typename... LambdaArgs>
struct compiles : std::false_type {};

template <typename Lambda, typename... LambdaArgs>
struct compiles<Lambda,
                std::void_t<decltype(std::declval<Lambda>()(
                    std::declval<LambdaArgs>()...))>,
                LambdaArgs...> : std::true_type {};
} // namespace compiles_detail

template <typename Lambda, typename... LambdaArgs>
struct compiles : compiles_detail ::compiles<Lambda, void, LambdaArgs...> {};

template <typename E, typename = void>
struct is_ast : std::false_type {};

template <typename E>
struct is_ast<E, std::void_t<typename E::__is_ast>> : std::true_type {};

template <typename... Ps>
using all_E_t = std::enable_if_t<std::conjunction_v<is_ast<Ps>...>>;

template <typename P1, typename P2>
struct And {
    using __is_ast = void;
    template <typename... T>
    static constexpr bool eval() {
        return P1::template eval<T...>() && P2::template eval<T...>();
    }
};

template <typename P1, typename P2>
struct Or {
    using __is_ast = void;
    template <typename... T>
    static constexpr bool eval() {
        return P1::template eval<T...>() || P2::template eval<T...>();
    }
};

template <typename P1>
struct Not {
    using __is_ast = void;
    template <typename... T>
    static constexpr bool eval() {
        return !P1::template eval<T...>();
    }
};

// leaf of ast
template <template <typename...> class Pred>
struct Id {
    using __is_ast = void;
    template <typename... T>
    static constexpr bool eval() {
        return Pred<T...>::value;
    }
};

template <typename P1, typename P2, typename = all_E_t<P1, P2>>
constexpr And<P1, P2> operator&&(P1, P2) {
    return And<P1, P2>();
}

template <typename P1, typename P2, typename = all_E_t<P1, P2>>
constexpr Or<P1, P2> operator||(P1, P2) {
    return Or<P1, P2>();
}

template <typename P1, typename = all_E_t<P1>>
constexpr Not<P1> operator!(P1) {
    return Not<P1>();
}

template <typename... T, typename AST>
constexpr bool constraint(AST) {
    return AST::template eval<T...>();
}

} // namespace detail

// all you need is pass a Pred into this, which act like std::is_pod
template <template <typename...> class Pred>
constexpr detail::Id<Pred> pred() {
    return detail::Id<Pred>();
}

template <typename T, auto &&AST>
using REQUIRES = std::enable_if_t<detail::constraint<T>(AST)>;

} // namespace constraint
