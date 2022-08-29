#include "constraint.h"
#include <type_traits>

// https://www.foonathan.net/2016/09/cpp14-concepts/

namespace intro {

template <typename T, typename vvv = void>
struct has_foo : std::false_type {};

template <typename T>
struct has_foo<T, std::void_t<decltype(std::declval<T>().foo())>>
    : std::true_type {};

template <typename T, typename = std::enable_if_t<has_foo<T>::value>>
auto CallFoo(T t) {
    t.foo();
}

int main() {

    int a;
    /* CallFoo(a); */
    struct fo {
        void foo() {}
    };
    fo wd;
    CallFoo(wd);

    return 0;
}

} // namespace intro

namespace concepts {

struct memory_block {};

template <typename T, template <typename...> class Expression,
          typename vvv = void>
struct compiles : std::false_type {};

template <typename T, template <typename...> class Expression>
struct compiles<T, Expression, std::void_t<Expression<T>>> : std::true_type {};

template <typename T, template <typename...> class Expression,
          typename ReturnType, typename vvv = void>
struct compiles_same_type
    : std::conjunction<std::is_same<ReturnType, Expression<T>>,
                       compiles<T, Expression>> {};

template <typename T, template <typename...> class Expression,
          typename ReturnType, typename vvv = void>
struct compiles_convertiable_type
    : std::conjunction<std::is_convertible<ReturnType, Expression<T>>,
                       compiles<T, Expression>> {};

template <typename T>
struct BlockAllocator_impl {
    template <typename Allocator>
    using allocate_block = decltype(std::declval<Allocator>().allocate_block());

    template <typename Allocator>
    using deallocate_block =
        decltype(std::declval<Allocator>().deallocate_block(
            std::declval<memory_block>()));

    template <typename Allocator>
    using next_block_size =
        decltype(std::declval<const Allocator>().next_block_size());

    using result =
        std::conjunction<compiles<T, deallocate_block>,
                         compiles_convertiable_type<T, next_block_size, size_t>,
                         compiles_same_type<T, allocate_block, memory_block>>;
};

template <typename T>
using BlockAllocator = typename BlockAllocator_impl<T>::result;

template <typename ResultType, typename CheckType,
          template <typename> typename... Concepts>
using REQUIRE =
    std::enable_if_t<std::conjunction_v<Concepts<CheckType>...>, ResultType>;

template <typename T>
auto alloc() -> REQUIRE<void, T, BlockAllocator> {}

void test_alloc() {
    struct wd {
        auto next_block_size() const -> int;
        auto deallocate_block(memory_block) -> void;
        auto allocate_block() -> memory_block;
    };
    alloc<wd>();
}
} // namespace concepts

using namespace constraint;

struct memory_block {};

constexpr auto isInt     = pred<BindTp<std::is_same, int>::type>();
constexpr auto isTrue_t  = pred<BindTp<std::is_same, std::true_type>::type>();
constexpr auto isPod     = pred<std::is_pod>();
constexpr auto isItegral = pred<std::is_integral>();
constexpr auto isLiteralType       = pred<std::is_literal_type>();
constexpr auto isTriviallyCopyable = pred<std::is_trivially_copyable>();

template <typename T>
struct isSame {
    template <typename E>
    struct type : std::is_same<T, E> {};
};

template <typename T>
struct BlockAllocator {
    static constexpr auto isMemoryBlock =
        pred<BindTp<std::is_same, memory_block>::type>();
    static constexpr auto isSizet = pred<BindTp<std::is_same, size_t>::type>();

    constexpr static auto deallocate_block =
        [](auto b, memory_block block =
                       memory_block{}) -> decltype(b.deallocate_block(block)) {
    };

    static constexpr auto next_block_size =
        [](const auto b) -> REQUIRES<decltype(b.next_block_size()), isSizet> {
    };

    constexpr static auto allocate_block =
        [](auto b) -> REQUIRES<decltype(b.allocate_block()), isMemoryBlock> {
    };

    static constexpr auto value =
        std::conjunction_v<should_compile_t(deallocate_block, T),
                           should_compile_t(next_block_size, T),
                           should_compile_t(allocate_block, T)>;
};

constexpr auto can_allocate = pred<BlockAllocator>();

constexpr auto fuck = isPod && can_allocate;

template <typename T, typename = REQUIRES<T, fuck>>
void need_fuck() {}

void test() {
    struct wd {
        void         deallocate_block(memory_block);
        memory_block allocate_block();
        size_t       next_block_size() const;
    };
    struct wdd {
        virtual ~wdd() {}
    };

    /* static_assert(isPod.eval<wd>()); */
    static_assert(can_allocate.eval<wd>());
    /* static_assert(isInt.eval<wd>()); */
    need_fuck<wd>;
    /* need_fuck<wdd>; */
}
