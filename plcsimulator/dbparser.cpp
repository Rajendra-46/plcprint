#include "dbparser.h"

namespace parser {

Parser<QString> comment()
{
    return sequence(token("//"), restOfLine());
}

Parser<> skipWhitespaceAndComments()
{
    return clearDiagnostics(dropValues(oneOf(comment(), map(skipWhitespace(), []() { return 0; }))));
}

Parser<QString> identifier()
{
    return sequence(skipWhitespaceAndComments(), word(), skipWhitespaceAndComments());
}

Parser<QString> singleQuoted()
{
    return sequence(skipWhitespaceAndComments(), quoted('\''), skipWhitespaceAndComments());
}

Parser<QString> doubleQuoted()
{
    return sequence(skipWhitespaceAndComments(), quoted('\"'), skipWhitespaceAndComments());
}

Parser<Property> property()
{
    auto parser = sequence(identifier(), token(":="), singleQuoted());

    return map(parser, [](QString const& name, QString const& value) {
        return Property{name, value};
    });
}

Parser<std::vector<Property>> properties()
{
    return map(optional(sequence(token("{"), separatedArray(property(), token(";")), token("}"))),
            [](std::optional<std::vector<Property>> properties) -> std::vector<Property> {
                if (properties)
                    return *properties;
                else
                    return std::vector<Property>();
            });
}

Parser<VariableType> primitiveType()
{
    QMap<QString, VariableType> typeMap = {{"Bool", VariableType::boolean},
            {"Byte", VariableType::byte},
            {"Int", VariableType::integer},
            {"UInt", VariableType::unsigned_integer},
            {"Char", VariableType::character},
            {"Word", VariableType::word},
            {"Real", VariableType::real}};

    return matchStrings(typeMap);
}

Parser<TypeReference> typeReference()
{
    return map(doubleQuoted(), [](QString const& str) { return TypeReference{str}; });
}

Parser<Array> array()
{
    return {[](QStringView str) {
        auto parser = sequence(token("Array"),
                token("["),
                integer(),
                token(".."),
                integer(),
                token("]"),
                token("of"),
                variableTypeDefinition());

        return map(parser,
                [](int low, int high, VariableTypeDefinition const& variableTypeDefinition) {
                    return Array{low,
                            high,
                            std::make_shared<VariableTypeDefinition>(variableTypeDefinition)};
                })(str);
    }};
}

Parser<VariableTypeDefinition> variableTypeDefinition()
{
    auto parser = oneOf(primitiveType(), structure(), typeReference(), array());

    return map(parser, [](std::variant<VariableType, Structure, TypeReference, Array> const& typeOrStruct) {
        if (std::holds_alternative<VariableType>(typeOrStruct))
        {
            return VariableTypeDefinition{std::get<VariableType>(typeOrStruct), std::monostate()};
        }
        else if (std::holds_alternative<Structure>(typeOrStruct))
        {
            return VariableTypeDefinition{
                    VariableType::structure, std::get<Structure>(typeOrStruct)};
        }
        else if (std::holds_alternative<TypeReference>(typeOrStruct))
        {
            return VariableTypeDefinition{VariableType::type, std::get<TypeReference>(typeOrStruct)};
        }
        else if (std::holds_alternative<Array>(typeOrStruct))
        {
            return VariableTypeDefinition{VariableType::array, std::get<Array>(typeOrStruct)};
        }
        else
        {
            assert(false);
            return VariableTypeDefinition();
        }
    });
}

Parser<VariableDefinition> variableDefinition()
{
    auto parser = sequence(optional(token("\"")),
            identifier(),
            optional(token("\"")),
            properties(),
            token(":"),
            variableTypeDefinition(),
            token(";"),
            skipWhitespaceAndComments());

    return map(parser,
            [](QString const& name,
                    std::vector<Property> const& properties,
                    VariableTypeDefinition const& variableTypeDefinition) {
                return VariableDefinition{name, properties, variableTypeDefinition};
            });
}

Parser<Structure> structure()
{
    return {[](QStringView str) -> ParseResult<Structure> {
        auto parser = sequence(token("STRUCT", Qt::CaseInsensitive),
                nOrMore(1, variableDefinition()),
                token("END_STRUCT"),
                skipWhitespaceAndComments());

        return map(parser, [](std::vector<VariableDefinition> variables) -> Structure {
            return Structure{variables};
        })(str);
    }};
}

Parser<Var> var()
{
    return {[](QStringView str) -> ParseResult<Var> {
        auto parser = sequence(token("VAR"),
                nOrMore(1, variableDefinition()),
                token("END_VAR"),
                skipWhitespaceAndComments());

        return map(parser, [](std::vector<VariableDefinition> variables) -> Var {
            return Var{{variables}};
        })(str);
    }};
}

Parser<TypeDefinition> typeDefinition()
{
    auto parser = sequence(token("TYPE"),
            doubleQuoted(),
            token("VERSION"),
            token(":"),
            integer(),
            token("."),
            integer(),
            structure(),
            token(";"),
            skipWhitespaceAndComments(),
            token("END_TYPE"),
            skipWhitespaceAndComments());

    return map(parser, [](QString name, int major, int minor, Structure structure) {
        return TypeDefinition{name, {major, minor}, structure};
    });
}

Parser<Version> version()
{
    return map(sequence(integer(), token("."), integer()), [](int major, int minor) {
        return Version{major, minor};
    });
}

Parser<Structure> structureOrVar()
{
    return map(oneOf(sequence(structure(), token(";")), var()),
            [](std::variant<Structure, Var> const& s) -> Structure {
                if (std::holds_alternative<Structure>(s))
                    return std::get<Structure>(s);
                else // if (std::holds_alternative<Var>(s))
                    return std::get<Var>(s);
            });
}

Parser<DataBlock> dataBlock()
{
    auto parser = sequence(token("DATA_BLOCK"),
            doubleQuoted(),
            optional(sequence(token("TITLE"), token("="), restOfLine())),
            properties(),
            token("VERSION"),
            token(":"),
            version(),
            optional(token("NON_RETAIN")),
            structureOrVar(),
            skipWhitespaceAndComments(),
            token("BEGIN"),
            token("END_DATA_BLOCK"));

    return map(parser,
            [](QString const& name,
                    std::optional<QString> const& title,
                    std::vector<Property> const& properties,
                    Version version,
                    Structure const& structure) -> DataBlock {
                return DataBlock{name, title.value_or(""), properties, version, structure};
            });
}

using VariableTypeVisitor = std::function<bool(QString const& path, VariableTypeDefinition const&)>;

void visitStructure(QMap<QString, TypeDefinition> const& types,
        QString const& path,
        Structure const& structure,
        VariableTypeVisitor visitor);

void visitVariableTypeDefinition(QMap<QString, TypeDefinition> const& types,
        QString const& path,
        VariableTypeDefinition const& v,
        VariableTypeVisitor visitor)
{
    if (visitor(path, v))
        return;

    if (v.type == VariableType::array)
    {
        Array const& array = std::get<Array>(v.structure);
        for (int i = array.low; i <= array.high; ++i)
            visitVariableTypeDefinition(types,
                    path + "[" + QString::number(i) + "]",
                    *std::get<Array>(v.structure).variable,
                    visitor);
    }
    else if (v.type == VariableType::structure)
    {
        visitStructure(types, path, std::get<Structure>(v.structure), visitor);
    }
    else if (v.type == VariableType::type)
    {
        visitor(path, v);

        TypeReference const& type = std::get<TypeReference>(v.structure);
        auto i = types.find(type.typeName);
        if (i == types.end())
            qWarning() << "Unable to find type " << type.typeName;
        else
            visitStructure(types, path, i->structure, visitor);
    }
}

void visitStructure(QMap<QString, TypeDefinition> const& types,
        QString const& path,
        Structure const& structure,
        VariableTypeVisitor visitor)
{
    for (VariableDefinition const& v : structure.variables)
    {
        visitVariableTypeDefinition(types, path + "." + v.name, v.type, visitor);
    }
}

void visitDataBlock(QMap<QString, TypeDefinition> const& types,
        DataBlock const& dataBlock,
        VariableTypeVisitor visitor)
{
    visitStructure(types, "root", dataBlock.structure, visitor);
}

Parser<DataBlockDocument> dataBlockDocument()
{
    auto parser = sequence(
            nOrMore(0, typeDefinition()), dataBlock(), skipWhitespaceAndComments(), endOfFile());
    return map(parser,
            [](std::vector<TypeDefinition> const& types,
                    DataBlock const& dataBlock) -> DataBlockDocument {
                QMap<QString, TypeDefinition> resultTypes;
                for (auto const& type : types)
                    resultTypes.insert(type.name, type);

                size_t dataBlockSize = 0;

                visitDataBlock(resultTypes,
                        dataBlock,
                        [&resultTypes, &dataBlockSize](
                                QString const& /*path*/, VariableTypeDefinition const& variable) {
                            switch (variable.type)
                            {
                            case parser::VariableType::boolean:
                                dataBlockSize += 1;
                                break;

                            case parser::VariableType::byte:
                            case parser::VariableType::character:
                                dataBlockSize += 8;
                                break;

                            case parser::VariableType::integer:
                            case parser::VariableType::unsigned_integer:
                            case parser::VariableType::word:
                                dataBlockSize += 16;
                                break;

                            case parser::VariableType::real:
                                dataBlockSize += 32;
                                break;

                            case parser::VariableType::unknown:
                            case parser::VariableType::structure:
                            case parser::VariableType::type:
                            case parser::VariableType::array:
                                break;
                            }

                            if (variable.type == VariableType::type)
                            {
                                if (resultTypes.find(
                                            std::get<TypeReference>(variable.structure).typeName)
                                        == resultTypes.end())
                                {
                                    qWarning()
                                            << "Unable to find type: "
                                            << std::get<TypeReference>(variable.structure).typeName;
                                }
                            }

                            return false;
                        });

                if (dataBlockSize % 8)
                    dataBlockSize += 8 - (dataBlockSize % 8);

                return DataBlockDocument{std::move(resultTypes), dataBlock, dataBlockSize / 8};
            });
}

} // namespace parser
