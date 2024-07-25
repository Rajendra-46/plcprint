#pragma once

#include <tuple>

namespace parser {

template<typename T>
auto tuplefy(T&& t)
{
    return std::make_tuple(std::forward<T>(t));
}

template<typename... Ts>
auto tuplefy(std::tuple<Ts...> t)
{
    return std::move(t);
}

template<typename T, typename U>
using TuplefyResult = decltype(
        std::tuple_cat(tuplefy(std::declval<T>()), tuplefy(std::declval<U>())));

} // namespace parser

