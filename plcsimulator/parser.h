#pragma once

#include "diagnostic.h"
#include "parseerror.h"
#include "parseresult.h"
#include "parsevalue.h"
#include "tuplereduce.h"

#include <exceptions.h>

#include <QDebug>
#include <QString>

#include <functional>
#include <type_traits>
#include <variant>

namespace parser {

template<typename... Ts>
class Parser
{
public:
    Parser(std::function<ParseResult<Ts...>(QStringView)> f) : m_f(std::move(f))
    {
    }

    Parser(Parser const&) = default;
    Parser(Parser&&) = default;

    Parser& operator=(Parser const&) = default;
    Parser& operator=(Parser&&) = default;

    ParseResult<Ts...> operator()(QStringView str) const
    {
        return m_f(str);
    }

private:
    std::function<ParseResult<Ts...>(QStringView)> m_f;
};

Parser<QChar> characterPredicate(std::function<bool(QChar)> predicate,
        QString expectedDiagnostic = "character");
Parser<QChar> letter();
Parser<QChar> whitespaceChar();
Parser<> optional(Parser<> parser);
QString vectorToQString(std::vector<QChar> vec);
Parser<> skipWhitespace();
Parser<QString> word();
Parser<> match(QString arg, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);
Parser<> token(QString arg, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);
Parser<QString> quoted(QChar mark = '\"');
Parser<int> integer();
Parser<QString> restOfLine();
Parser<> endOfLine();
Parser<> endOfFile();

namespace detail {
template<typename T>
struct ParserHelper
{
    using type = Parser<T>;
};

template<>
struct ParserHelper<void>
{
    using type = Parser<>;
};

template<typename... Ts>
struct ParserHelper<ParseResult<Ts...>>
{
    using type = Parser<Ts...>;
};

template<typename T>
struct ParseResultHelper
{
    using type = ParseResult<T>;
};

template<>
struct ParseResultHelper<void>
{
    using type = ParseResult<>;
};

template<typename... Ts>
struct ParseResultHelper<ParseResult<Ts...>>
{
    using type = ParseResult<Ts...>;
};
} // namespace detail

template<typename TFunc, typename... Ts>
auto map(Parser<Ts...> parser, TFunc&& func) ->
        typename detail::ParserHelper<std::invoke_result_t<TFunc, Ts...>>::type
{
    using InvokeType = std::invoke_result_t<TFunc, Ts...>;
    using ResultType = typename detail::ParseResultHelper<InvokeType>::type;
    return {[func = std::forward<TFunc>(func), parser = std::move(parser)](
                    QStringView str) -> ResultType {
        auto r = parser(str);
        if (r.hasValue())
        {
            if constexpr (std::is_same_v<void, InvokeType>)
            {
                std::apply(func, r.values());
                return makeParseResult(std::tuple<>(), r.remainder(), r.diagnostics());
            }
            else
            {
                return makeParseResult(
                        tuplefy(std::apply(func, r.values())), r.remainder(), r.diagnostics());
            }
        }
        else
        {
            return ParseError(r.error());
        }
    }};
}

template<typename... Ts>
auto dropValues(Parser<Ts...> parser)
{
    return map(parser, [](auto&&...) {});
}

template<typename T>
Parser<std::vector<T>> nOrMore(int n, Parser<T> parser)
{
    return {[parser = std::move(parser), n](QStringView str) -> ParseResult<std::vector<T>> {
        std::vector<T> result;

        QStringView remainder = str;
        ParseResult<T> r = parser(remainder);
        int m = 0;

        while (r.hasValue())
        {
            result.push_back(r.template value<0>());
            ++m;
            remainder = r.remainder();
            r = parser(remainder);
        }

        if (n <= m)
        {
            return ParseValue(tuplefy(std::move(result)), remainder, r.diagnostics());
        }
        else
            return ParseError(r.error());
    }};
}

template<typename T>
Parser<std::optional<T>> optional(Parser<T> parser)
{
    return {[parser = std::move(parser)](QStringView str) -> ParseResult<std::optional<T>> {
        ParseResult<T> r = parser(str);

        if (r.hasValue())
        {
            return makeParseResult(tuplefy(std::make_optional(r.template value<0>())),
                    r.remainder(),
                    r.diagnostics());
        }

        return makeParseResult(tuplefy(std::optional<T>(std::nullopt)), str, r.diagnostics());
    }};
}

template<typename... Ts, typename... Us>
auto sequence(Parser<Ts...> parser1, Parser<Us...> parser2) -> Parser<Ts..., Us...>
{
    return {[parser1, parser2](QStringView str) -> ParseResult<Ts..., Us...>{auto r = parser1(str);
    if (!r.hasValue())
        return ParseError(r.error());

    auto r2 = parser2(r.remainder());

    if (!r2.hasValue())
    {
        ParseError result;
        result.appendDiagnostics(r2.diagnostics());
        result.appendDiagnostics(r.diagnostics());

        return result;
    }

    return makeParseResult(
            std::tuple_cat(r.values(), r2.values()), r2.remainder(), r2.diagnostics());
}
}; // namespace parser
}

template<typename... Ts, typename... Us, typename... Vs, typename... Ws>
auto sequence(Parser<Ts...> parser1, Parser<Us...> parser2, Parser<Vs...> parser3, Ws&&... rest)
{
    return sequence(sequence(parser1, parser2), parser3, std::forward<Ws>(rest)...);
}

template<typename... Ts>
Parser<Ts...> clearDiagnostics(Parser<Ts...> parser)
{
    return {[parser](QStringView str) -> ParseResult<Ts...>{auto r = parser(str);
    if (r.hasValue())
        return makeParseResult(r.values(), r.remainder(), {});
    else
        return r.error();
}
}
;
}

template<typename T>
Parser<std::vector<T>> separatedArray(Parser<T> valueParser, Parser<> separatorParser)
{
    return {[valueParser, separatorParser](QStringView str) -> ParseResult<std::vector<T>> {
        std::vector<T> result;

        QStringView remainder = str;
        ParseResult<T> r = valueParser(remainder);

        while (r.hasValue())
        {
            result.push_back(r.template value<0>());

            ParseResult<> r2 = separatorParser(r.remainder());
            if (!r2.hasValue())
            {
                return ParseValue(tuplefy(std::move(result)), r.remainder(), r2.diagnostics());
            }

            r = valueParser(r2.remainder());

            if (!r.hasValue())
                return ParseError(r.error());

            remainder = r.remainder();
        }

        return ParseValue(tuplefy(std::move(result)), remainder, r.diagnostics());
    }};
}

template<typename... Ts>
Parser<std::variant<Ts...>> oneOf(Parser<Ts>... parsers)
{
    return {[parsers = std::make_tuple(std::move(parsers)...)](QStringView str)
                    -> ParseResult<std::variant<Ts...>>{
                            ParseResult<std::variant<Ts...>> r = tupleReduce(
                                    ParseResult<std::variant<Ts...>>(ParseError(
                                            std::nullopt, "No match found, tried following:")),
                                    parsers,
                                    [str](ParseResult<std::variant<Ts...>> current,
                                            auto parser) -> ParseResult<std::variant<Ts...>> {
                                        if (current.hasValue())
                                            return current;

                                        auto r = parser(str);
                                        if (!r.hasValue())
                                        {
                                            ParseError err(current.error());
                                            err.appendDiagnostics(r.diagnostics());
                                            return err;
                                        }

                                        return makeParseResult(
                                                tuplefy(std::variant<Ts...>(r.template value<0>())),
                                                r.remainder());
                                    });

    return r;
}
}
;
}

template<typename T>
Parser<T> matchStrings(QMap<QString, T> map)
{
    return {[map](QStringView str) -> ParseResult<T> {
        for (auto i = map.begin(); i != map.end(); ++i)
        {
            if (str.startsWith(i.key()))
                return makeParseResult(i.value(), str.right(str.size() - i.key().size()));
        }

        QString err = QString("Expected one of %1").arg(map.keys().join(", "));

        return ParseError(str, err);
    }};
}

class ParseException : public libros::StartupException
{
public:
    ParseException(const QString& diagnostic) : libros::StartupException(diagnostic)
    {
    }
};

template<typename T>
T parse(Parser<T> parser, QStringView str)
{
    auto r = parser(str);

    if (!r.hasValue())
        throw ParseException(r.detailed(str));

    return r.template value<0>();
}

void parse(Parser<> parser, QStringView str);

template<typename T, typename U, typename... Vs>
std::tuple<T, U, Vs...> parse(Parser<T, U, Vs...> parser, QStringView str)
{
    auto r = parser(str);

    if (!r.hasValue())
        throw ParseException(r.detailed(str));

    return r.values();
}

} // namespace parser
