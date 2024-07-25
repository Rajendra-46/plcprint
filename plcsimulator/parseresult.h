#pragma once

#include "diagnostic.h"
#include "parseerror.h"
#include "parsevalue.h"
#include "tuplefy.h"

#include <QString>
#include <QStringView>

#include <tuple>
#include <variant>

namespace parser {

template<typename... Ts>
class ParseResult;

template<typename... Ts>
auto makeParseResult(std::tuple<Ts...> value,
        QStringView remainder,
        std::vector<Diagnostic> diagnostics = {})
{
    return ParseResult<std::decay_t<Ts>...>(
            ParseValue<std::decay_t<Ts>...>(std::move(value), remainder, std::move(diagnostics)));
    ;
}

template<typename T>
auto makeParseResult(T&& t, QStringView remainder, std::vector<Diagnostic> diagnostics = {})
{
    return makeParseResult(tuplefy(std::forward<T>(t)), remainder, std::move(diagnostics));
}

template<typename... Ts>
class ParseResult
{
public:
    ParseResult(ParseValue<Ts...> value) : m_result(std::move(value))
    {
    }

    ParseResult(ParseError error) : m_result(std::move(error))
    {
    }

    bool isFailure() const
    {
        return std::holds_alternative<ParseError>(m_result);
    }

    bool hasValue() const
    {
        return std::holds_alternative<ParseValue<Ts...>>(m_result);
    }

    template<size_t Index>
    auto value() const -> std::tuple_element_t<Index, std::tuple<Ts...>> const&
    {
        return std::get<ParseValue<Ts...>>(m_result).template value<Index>();
    }

    std::tuple<Ts...> const& values() const
    {
        return std::get<ParseValue<Ts...>>(m_result).values();
    }

    QStringView remainder() const
    {
        return std::get<ParseValue<Ts...>>(m_result).remainder();
    }

    ParseError const& error() const
    {
        return std::get<ParseError>(m_result);
    }

    QString detailed(QStringView original) const
    {
        if (hasValue())
            return "";

        return error().detailed(original);
    }

    QStringView content() const
    {
        return std::get<ParseError>(m_result).content();
    }

    std::vector<Diagnostic> diagnostics() const
    {
        if (hasValue())
            return std::get<ParseValue<Ts...>>(m_result).diagnostics();
        else
            return error().diagnostics();
    }

private:
    std::variant<ParseValue<Ts...>, ParseError> m_result;
};

} // namespace parser

