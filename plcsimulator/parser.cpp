#include "parser.h"

namespace parser {
Parser<QChar> characterPredicate(std::function<bool(QChar)> predicate, QString expectedDiagnostic)
{
    return {[predicate = std::move(predicate), expectedDiagnostic = std::move(expectedDiagnostic)](
                    QStringView str) -> ParseResult<QChar> {
        if (str.isEmpty())
            return ParseError(str, QString("Unexpected end of file"));

        if (!predicate(str[0]))
        {
            return ParseError(
                    str, QString("Found '%1', expected %2").arg(str[0]).arg(expectedDiagnostic));
        }

        return ParseValue<QChar>(str[0], str.right(str.size() - 1));
    }};
}

Parser<QChar> letter()
{
    return characterPredicate([](QChar ch) { return ch.isLetter(); }, "a letter");
}

Parser<QChar> letterOrNumber()
{
    return characterPredicate([](QChar ch) { return ch.isLetter() || ch.isNumber() || ch == '_'; },
            "a letter or a digit");
}

Parser<QChar> whitespaceChar()
{
    return characterPredicate([](QChar ch) { return ch.isSpace(); }, "whitespace");
}

Parser<> optional(Parser<> parser)
{
    return {[parser = std::move(parser)](QStringView str) -> ParseResult<> {
        ParseResult<> r = parser(str);

        if (!r.hasValue())
            return makeParseResult(std::tuple<>(), str, r.diagnostics());

        return makeParseResult(std::tuple<>(), r.remainder(), r.diagnostics());
    }};
}

QString vectorToQString(std::vector<QChar> vec)
{
    QString result;
    for (auto v : vec)
        result.append(v);

    return result;
}

Parser<> skipWhitespace()
{
    return clearDiagnostics(dropValues(map(nOrMore(0, whitespaceChar()), vectorToQString)));
}

Parser<QString> word()
{
    return map(nOrMore(1, letterOrNumber()), vectorToQString);
}

Parser<> match(QString arg, Qt::CaseSensitivity caseSensitivity)
{
    return {[arg, caseSensitivity](QStringView str) -> ParseResult<> {
        if (str.size() < arg.size())
            return ParseError(str.right(str.size()), "Unexpected end of file");

        if (!str.startsWith(arg, caseSensitivity))
            return ParseError(str, QString("expected: \"%1\"").arg(arg));

        return makeParseResult(std::tuple<>(), str.right(str.size() - arg.size()));
    }};
}

Parser<> token(QString arg, Qt::CaseSensitivity caseSensitivity)
{
    return sequence(skipWhitespace(), match(arg, caseSensitivity), skipWhitespace());
}

Parser<QString> quoted(QChar mark)
{
    return {[mark](QStringView str) -> ParseResult<QString> {
        if (str.size() < 1)
            return ParseError(str, "Unexpected end of file");

        if (str[0] != mark)
            return ParseError(str, QString("Expected '%1' got '%2'").arg(mark).arg(str[0]));

        int index = 1;
        QString result;
        while (true)
        {
            if (str.size() <= index)
            {
                return ParseError(str.right(str.size() - 1), "Unexpected end of file");
            }
            if (str[index] == "\\" && str.size() > (index + 1) && str[index + 1] == mark)
            {
                index += 2;
            }
            else if (str[index] == mark)
            {
                ++index;
                break;
            }
            else
            {
                result.append(str[index]);
                ++index;
            }
        }

        return makeParseResult(std::make_tuple(result), str.right(str.size() - index));
    }};
}

Parser<int> integer()
{
    return map(nOrMore(1, characterPredicate([](QChar ch) { return ch.isNumber(); }, "integer")),
            [](std::vector<QChar> v) {
                int r = 0;
                for (auto ch : v)
                {
                    r *= 10;
                    r += ch.digitValue();
                }

                return r;
            });
}

Parser<QString> restOfLine()
{
    return {[](QStringView str) -> ParseResult<QString> {
        if (str.empty())
            return makeParseResult(tuplefy(QString("")), str);

        int index = 0;
        auto i = str.begin();
        while (i != str.end() && *i != '\n')
        {
            ++i;
            ++index;
        }

        if (i == str.end())
            return makeParseResult(tuplefy(str.toString()), str.right(0));

        return makeParseResult(
                tuplefy(str.left(index).toString()), str.right(str.size() - index - 1));
    }};
}

Parser<> endOfLine()
{
    return {[](QStringView str) -> ParseResult<> {
        if (str.empty())
            return makeParseResult(std::tuple<>(), str);
        else if (str[0] == '\n')
            return makeParseResult(std::tuple<>(), str.right(str.size() - 1));

        return ParseError(str, "Expected end of line");
    }};
}

Parser<> endOfFile()
{
    return {[](QStringView str) -> ParseResult<> {
        if (str.empty())
            return makeParseResult(std::tuple<>(), str);

        return ParseError(str, "Expected end of file");
    }};
}

void parse(Parser<> parser, QStringView str)
{
    auto r = parser(str);

    if (!r.hasValue())
        throw ParseException(r.detailed(str));
}


} // namespace parser
