/*
    Visit std::tuple at given index at runtime, like std::visit for std::variant (C++17)
*/

#pragma once

#include <array>
#include <functional>
#include <tuple>
#include <type_traits>

namespace detail {

template <class... Ts>
struct IdenticalTypeHelper;

template <class T, class... Ts>
struct IdenticalTypeHelper<T, Ts...> {
    using Type = std::enable_if_t<(std::is_same_v<T, Ts> && ...), T>;
};

template <class... Ts>
using IdenticalType = typename IdenticalTypeHelper<Ts...>::Type;

template <class Tuple>
using TupleIndices = std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>;

template <class Visior, class Tuple, class IndexSequence>
struct VisitResultHelper;

template <class Visitor, class Tuple, std::size_t... Indices>
struct VisitResultHelper<Visitor, Tuple, std::index_sequence<Indices...>> {
    using Type = IdenticalType<std::invoke_result_t<Visitor, decltype(std::get<Indices>(std::declval<Tuple>()))>...>;
};

template <class Visitor, class Tuple>
using VisitResult = typename VisitResultHelper<Visitor, Tuple, TupleIndices<Tuple>>::Type;

template <class Visitor, class Tuple>
using VisitFunc = VisitResult<Visitor, Tuple>(*)(Visitor&& visitor, Tuple&& tuple);

template <std::size_t Index, class Visitor, class Tuple>
VisitResult<Visitor, Tuple> VisitIndex(Visitor&& visitor, Tuple&& tuple) {
    return std::invoke(std::forward<Visitor>(visitor), std::get<Index>(std::forward<Tuple>(tuple)));
}

template<class Visitor, class Tuple, std::size_t... Indices>
constexpr auto PrepareVisitHelpers(std::index_sequence<Indices...>) {
    return std::array<VisitFunc<Visitor, Tuple>, sizeof...(Indices)>{VisitIndex<Indices, Visitor, Tuple>...};
}

template <class Visitor, class Tuple>
VisitResult<Visitor, Tuple> Visit(Visitor&& visitor, Tuple&& tuple, const std::size_t index) {
    static constexpr auto helpers = PrepareVisitHelpers<Visitor, Tuple>(TupleIndices<Tuple>{});
    return helpers[index](std::forward<Visitor>(visitor), std::forward<Tuple>(tuple));
}

} // namespace detail


using detail::Visit;
using detail::VisitResult;
