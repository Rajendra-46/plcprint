#pragma once

#include "parser.h"

#include <memory>
#include <variant>

namespace parser {

struct Property
{
    QString name;
    QString value;

    bool operator==(Property const& rhs) const
    {
        return name == rhs.name && value == rhs.value;
    }
};

enum class VariableType
{
    unknown,
    boolean,
    byte,
    integer,
    unsigned_integer,
    character,
    word,
    real,
    structure,
    type,
    array
};

struct VariableDefinition;
struct VariableTypeDefinition;

struct Structure
{
    std::vector<VariableDefinition> variables;
};

struct Var : Structure
{
};

struct TypeReference
{
    QString typeName;
};

struct Array
{
    int low;
    int high;
    std::shared_ptr<VariableTypeDefinition> variable;
};

struct VariableTypeDefinition
{
    VariableType type;
    std::variant<std::monostate, Structure, TypeReference, Array> structure;
};

struct VariableDefinition
{
    QString name;
    std::vector<Property> properties;
    VariableTypeDefinition type;
};

struct Version
{
    int major;
    int minor;
};

struct TypeDefinition
{
    QString name;
    Version version;
    Structure structure;
};

struct DataBlock
{
    QString name;
    QString title;
    std::vector<Property> properties;
    Version version;
    Structure structure;
};

struct DataBlockDocument
{
    QMap<QString, TypeDefinition> types;
    DataBlock dataBlock;

    // Datablock size in bits
    size_t size;
};

Parser<QString> comment();
Parser<> skipWhitespaceAndComments();
Parser<QString> identifier();
Parser<QString> singleQuoted();
Parser<QString> doubleQuoted();
Parser<Property> property();
Parser<std::vector<Property>> properties();
Parser<VariableType> primitiveType();
Parser<TypeReference> typeReference();
Parser<Structure> structure();
Parser<Var> var();
Parser<Array> array();
Parser<VariableTypeDefinition> variableTypeDefinition();
Parser<VariableDefinition> variableDefinition();
Parser<TypeDefinition> typeDefinition();
Parser<DataBlock> dataBlock();
Parser<DataBlockDocument> dataBlockDocument();

} // namespace parser
