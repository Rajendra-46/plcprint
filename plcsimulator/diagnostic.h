#pragma once

#include <QStringView>
#include <QString>

#include <optional>

namespace parser {

class Diagnostic
{
public:
    Diagnostic(QString diagnostic, std::optional<QStringView> context) :
        m_diagnostic(diagnostic),
        m_context(context)
    {
    }

    QString const& diagnostic() const
    {
        return m_diagnostic;
    }

    QStringView context() const
    {
        return *m_context;
    };

    bool hasContext() const
    {
        return m_context.has_value();
    }

    QString detailed(QStringView original) const
    {
        if (!m_context)
            return diagnostic();

        assert(original.data() <= m_context->data()
                && m_context->data() <= (original.data() + original.size()));

        int line = 1;
        int column = 1;
        auto lineStart = original.begin();

        for (auto i = original.begin(); i != original.end(); ++i)
        {
            QChar const& ch = *i;

            if (&ch == m_context->data())
            {
                break;
            }
            else if (ch == '\n')
            {
                ++line;
                column = 1;
                lineStart = i + 1;
            }
            else if (ch == '\r' && column == 1)
            {
                lineStart = i + 1;
                continue;
            }
            else
            {
                ++column;
            }
        }

        auto lineEnd = lineStart;
        while (lineEnd != original.end() && *lineEnd != '\n')
            ++lineEnd;

        QString lineStr;

        lineStr.append(QString("\nline %1: %2\n").arg(line).arg(diagnostic()));

        for (auto i = lineStart; i != lineEnd; ++i)
            lineStr.append(*i);

        lineStr.append('\n');

        for (int i = 1; i < column; ++i)
            lineStr.append(" ");

        lineStr.append(QString("^"));

        return lineStr;
    }

private:
    QString m_diagnostic;
    std::optional<QStringView> m_context;
};

} // namespace parser

