#pragma once

#include <QStringView>

#include <vector>
#include <tuple>

namespace parser {

class Diagnostic;

template<typename... Ts>
class ParseValue
{
public:
    ParseValue(std::tuple<Ts...> values,
            QStringView remainder,
            std::vector<Diagnostic> diagnostics = {}) :
        m_remainder(remainder),
        m_values(std::move(values)),
        m_diagnostics(std::move(diagnostics))
    {
    }

    QStringView remainder() const
    {
        return m_remainder;
    }

    template<size_t Index>
    auto value() const -> std::tuple_element_t<Index, std::tuple<Ts...>> const&
    {
        return std::get<Index>(m_values);
    }

    std::tuple<Ts...> const& values() const
    {
        return m_values;
    }

    std::vector<Diagnostic> const& diagnostics() const
    {
        return m_diagnostics;
    }

private:
    QStringView m_remainder;
    std::tuple<Ts...> m_values;
    std::vector<Diagnostic> m_diagnostics;
};

} // namespace parser

