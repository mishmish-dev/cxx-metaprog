/*
    Reverse template args in C++14
*/

#include <tuple>
#include <type_traits>

namespace detail {

template <class... Ts>
struct Tag{};

template <class... Ts>
struct Append{};

template <class T, class... Ts>
struct Append<T, Tag<Ts...>> {
    using Type = Tag<Ts..., T>;
};

template <class... Ts>
struct Revert {
    using Type = Tag<>;
};

template <class T, class... Ts>
struct Revert<T, Ts...> {
    using Type = typename Append<T, typename Revert<Ts...>::Type>::Type;
};

template <template <class...> class Template, class T>
struct ApplyTemplate{};

template <template <class...> class Template, class... Args>
struct ApplyTemplate<Template, Tag<Args...>> {
    using Type = Template<Args...>;
};

template <template <class...> class Template, class... Ts>
using ReverseTemplateArgs =
    typename detail::ApplyTemplate<Template, typename detail::Revert<Ts...>::Type>::Type;

} // namespace detail

using detail::ReverseTemplateArgs;

// Usage
template <class... Ts>
using ReverseTuple = ReverseTemplateArgs<std::tuple, Ts...>;

// Test
static_assert(std::is_same< std::tuple<bool, int, void*>, ReverseTuple<void*, int, bool> >::value, "T__T");