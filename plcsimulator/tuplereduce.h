#pragma once

#include <tuple>

namespace parser
{
    namespace detail
    {
        template <typename TFunc, typename T>
        struct Reducer
        {
            template <typename U>
            auto operator<<(U&& u) &&
            -> Reducer<
                TFunc,
                std::result_of_t<TFunc&(std::decay_t<T>&&, U&&)>
                >
            {
                auto v = func(std::move(value), std::forward<U>(u));

                return { func, std::move(v) };
            }

            TFunc& func;
            std::decay_t<T> value;
        };

        template <typename TFunc, typename U, typename TTuple, size_t... S>
        auto tupleReduceSeq(TFunc&& f, U&& initial, TTuple&& data,
                std::index_sequence<S...>)
        -> std::decay_t<decltype(
            std::move((
                        Reducer<std::decay_t<TFunc>, U>{
                            f,
                            std::forward<U>(initial)}
                        << ...
                        << std::get<S>(std::forward<TTuple>(data))
                        ).value
                    )
            )>
        {
            return std::move((
                        Reducer<std::decay_t<TFunc>, U>{
                            f,
                            std::forward<U>(initial)}
                        << ...
                        << std::get<S>(std::forward<TTuple>(data))
                        ).value
                    );
        }
    }

    template <typename TFunc, typename T, typename TTuple>
    auto tupleReduce(T&& initial, TTuple&& tuple, TFunc&& func)
    -> decltype(
            detail::tupleReduceSeq(
                std::forward<TFunc>(func),
                std::forward<T>(initial),
                std::forward<TTuple>(tuple),
                std::make_index_sequence<std::tuple_size<
                    std::decay_t<TTuple>>::value
                    >()
                )
            )
    {
        return detail::tupleReduceSeq(
                std::forward<TFunc>(func),
                std::forward<T>(initial),
                std::forward<TTuple>(tuple),
                std::make_index_sequence<std::tuple_size<
                    std::decay_t<TTuple>>::value
                    >()
                );
    }
} // parser

