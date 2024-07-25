#pragma once

#include "diagnostic.h"

#include <QStringView>
#include <QString>

#include <vector>
#include <optional>

namespace parser
{

class ParseError
{
public:
    ParseError()
    {
    }

    ParseError(std::optional<QStringView> context, QString diagnostic) :
        m_diagnostics({ Diagnostic(std::move(diagnostic), context)})
    {
    }

    std::vector<Diagnostic> const& diagnostics() const
    {
        return m_diagnostics;
    }

    QString detailed(QStringView original) const
    {
        QString result;

        for (Diagnostic const& d : diagnostics())
        {
            result.append("\n");
            result.append(d.detailed(original));
        }

        return result;
    }

    void appendDiagnostic(Diagnostic diagnostic)
    {
        m_diagnostics.push_back(diagnostic);
    }

    void appendDiagnostics(std::vector<Diagnostic> const& diagnostics)
    {
        for (Diagnostic const& d : diagnostics)
        {
            m_diagnostics.push_back(d);
        }
    }

private:
    std::vector<Diagnostic> m_diagnostics;
};

} // namespace parser

