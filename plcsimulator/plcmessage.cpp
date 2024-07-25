#include "plcmessage.h"

static const QStringList OUTGOING = {"ROS->SERVER", "CRANE->SERVER", "YARD->SERVER"};
static const QStringList INCOMING = {"SERVER->ROS", "SERVER->CRANE", "SERVER->YARD"};

PLCMessage::PLCMessage(const QString& dbPath, const QString& id) : m_id(id), m_dbPath(dbPath)
{
    parseDataBlock(m_dbPath);
    parseMessageBytes();
}

using VariableTypeVisitor
        = std::function<bool(QString const& path, parser::VariableTypeDefinition const&)>;

void visitDataBlock(QMap<QString, parser::TypeDefinition> const& types,
        parser::DataBlock const& dataBlock,
        VariableTypeVisitor visitor);

void PLCMessage::parseDataBlock(const QString& filename)
{
    auto str = readFileStr(filename);
    parser::Parser<parser::DataBlockDocument> parser = parser::dataBlockDocument();
    parser::ParseResult<parser::DataBlockDocument> result = parser(str);

    const QVector<QString> types = {"unknown",
            "boolean",
            "byte",
            "integer",
            "unsigned_integer",
            "character",
            "word",
            "real",
            "structure",
            "type",
            "array"};

    const QVector<QString> typeFilter = {"unknown", "structure", "type", "array"};

    QVector<QPair<QString, QString>> db;
    visitDataBlock(result.value<0>().types,
            result.value<0>().dataBlock,
            [&db = db, &types = types, &typeFilter = typeFilter](
                    QString const& path, parser::VariableTypeDefinition const& var) {
                auto type = types.at(int(var.type));
                if (!typeFilter.contains(type))
                {
                    db.push_back(QPair<QString, QString>(path, type));
                }
                return false;
            });

    Entry subData = Entry();
    QString lastIdentifier = "";

    for (auto& line : db)
    {
        auto ids = line.first.split(".");
        auto type = line.second;

        auto identifier = ids.at(1);
        if (identifier != lastIdentifier)
        {
            if (!subData.second.isEmpty())
            {
                m_dataStructure.push_back(subData);
            }
            subData.first = identifier;
            subData.second.clear();
            lastIdentifier = identifier;
        }

        if (ids.length() == 5)
        {
            subData.second.push_back(
                    QPair(QString("%0.%1").arg(ids.at(3), ids.at(4)), QVariant(type)));
        }
        else if (ids.length() == 4)
        {
            subData.second.push_back(QPair(QString("%0").arg(ids.at(3)), QVariant(type)));
        }
        else if (ids.length() == 3)
        {
            subData.second.push_back(QPair(QString("%0").arg(ids.at(2)), QVariant(type)));
        }
    }

    if (!subData.second.isEmpty())
    {
        m_dataStructure.push_back(subData);
    }

    for (auto& e : m_dataStructure)
    {
        for (auto& b : e.second)
        {
            auto type = b.second.toString();
            if (type == "boolean")
            {
                b.second = QVariant(bool(0));
            }
            else if (type == "byte")
            {
                b.second = QVariant(QByteArray(1, '0'));
            }
            else if (type == "integer")
            {
                b.second = QVariant(0);
            }
            else if (type == "unsigned_integer")
            {
                b.second = QVariant(0U);
            }
            else if (type == "character")
            {
                b.second = QVariant(QChar(0));
            }
            else if (type == "real")
            {
                b.second = QVariant(float(0.0));
            }
        }
    }
}

void PLCMessage::enableLogging(std::shared_ptr<TestLogger> logger, bool diff)
{
    m_logger = logger;
    m_differentialLogging = diff;
}

void PLCMessage::enableConfigServer(const QString& address)
{
    auto ip = QHostAddress(address.split(":").first());
    auto port = address.split(":").last().toInt();

    if (ip.isNull() || port <= 0 || port > 65535)
    {
        qCritical() << QString("[ERROR] Ivalid test server address: %0").arg(address);
        exit(0);
    }

    m_configurationServer = std::make_shared<QTcpServer>();

    m_configurationServer->connect(m_configurationServer.get(),
            &QTcpServer::newConnection,
            this,
            &PLCMessage::processHTTPMessage);

    m_configurationServer->listen(ip, port);

    if (!m_configurationServer->isListening())
    {
        qCritical() << QString("[ERROR] Cant open test server to: %0").arg(address);
        exit(0);
    }
}

void PLCMessage::processHTTPMessage()
{
    auto socket = m_configurationServer->nextPendingConnection();
    auto sender = socket->peerAddress().toString();
    socket->waitForReadyRead();
    auto data = socket->readAll();
    auto body = QString(data).split("\r\n\r\n").last();
    auto request = QJsonDocument::fromJson(body.toUtf8()).object();
    QString response;

    auto method = request.value("command").toString();

    if (method == "file")
    {
        auto fileName = request.value("file").toString().trimmed();
        auto file = QFile(fileName);

        if (!file.open(QIODevice::ReadOnly))
        {
            response = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\nError opening file: "
                    + fileName + "\n";
        }
        else
        {
            file.close();
            addUserSettings(fileName);
            emit bytesReceived();
            response = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n";
        }
    }
    else if (method == "value")
    {
        auto parentName = request.value("parentName").toString().trimmed();
        auto valueName = request.value("valueName").toString().trimmed();
        auto newValue = request.value("value");
        auto oldValue = value(parentName, valueName);

        if (!oldValue.isValid())
        {
            response = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\nCant find value for: "
                    + parentName + "." + valueName + "\n";
        }

        else
        {
            if (oldValue.type() == QVariant::Bool)
            {
                setValue(parentName, valueName, newValue.toBool());
            }

            else if (oldValue.type() == QVariant::ByteArray)
            {
                QByteArray array;
                array.setNum(newValue.toInt());
                setValue(parentName, valueName, array);
            }

            else if (oldValue.type() == QVariant::Int)
            {
                setValue(parentName, valueName, newValue.toInt());
            }

            else if (oldValue.type() == QVariant::Double)
            {
                setValue(parentName, valueName, newValue.toDouble());
            }

            else if (oldValue.type() == QVariant::Char)
            {
                setValue(parentName, valueName, newValue.toInt());
            }
            parseMessageBytes();
            response = "HTTP/1.1 200 OK\r\nConnection:close\r\n";
        }
    }

    else
    {
        response = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\nUnkown command: " + method
                + "\n";
    }

    socket->write(response.toUtf8());
    socket->close();
}

void PLCMessage::addUserSettings(const QString& jsonPath)
{
    QRegularExpression reQuotes("\"(.*?)\"");
    QRegularExpression reInteger("^\\d+$");
    QRegularExpression reDouble("\\-?\\d+\\.\\d+");

    auto lines = readFile(jsonPath);
    auto json = QJsonDocument::fromJson(lines.join("").toUtf8());

    if (json.isNull())
    {
        std::cout << "Error in user settings JSON file.\n"
                  << "Using default values.\n";
        return;
    }

    do
    {
        lines.removeLast();
    } while (lines.last().replace(" ", "").isEmpty());

    do
    {
        lines.removeFirst();
    } while (lines.first().replace(" ", "").isEmpty());

    for (auto index = 0; index < lines.length(); ++index)
    {
        auto cleanChars = QStringList() << " "
                                        << "\n"
                                        << ","
                                        << "\"";
        auto line = lines[index];
        auto header = line.split(":").at(0);

        foreach (auto c, cleanChars)
        {
            header.replace(c, "");
        }

        if (line.contains("{"))
        {
            auto startOfBraces = index;
            auto endOfBraces = ++index;

            while (!lines.at(endOfBraces).contains("}"))
            {
                ++endOfBraces;
            }

            for (auto stepper = ++startOfBraces; stepper < endOfBraces; ++stepper)
            {
                auto line = lines.at(stepper);
                foreach (auto c, cleanChars)
                {
                    line.replace(c, "");
                }
                auto parts = line.split(":");
                auto valueName = parts.first();

                auto value = parts.last();
                auto existingValue = PLCMessage::value(header, valueName);
                auto existingType = existingValue.type();

                if (existingValue.type() == QVariant::Invalid)
                {
                    std::cout << "User settings JSON file "
                                 "had non exsistent value"
                              << header.toStdString() << valueName.toStdString() << "\n\n";
                }

                else if (existingType == QVariant::Bool)
                {
                    setValue(header, valueName, QVariant(value == "true"));
                }

                else if (existingType == QVariant::ByteArray)
                {
                    auto byte = QByteArray();
                    byte.setNum(value.toInt());
                    setValue(header, valueName, QVariant(byte));
                }

                else if (existingType == QVariant::Int)
                {
                    setValue(header, valueName, QVariant(value.toInt()));
                }

                else if (existingType == QVariant::UInt)
                {
                    setValue(header, valueName, QVariant(value.toUInt()));
                }

                else if (existingType == QVariant::Double)
                {
                    setValue(header, valueName, QVariant(value.toDouble()));
                }

                else if (existingType == QVariant::Char)
                {
                    if (value.isEmpty())
                    {
                        setValue(header, valueName, QVariant(QChar(QChar::Null)));
                    }
                    else
                    {
                        setValue(header, valueName, QVariant(QChar(value.at(0).toLatin1())));
                    }
                }
            }
        }
    }
    parseMessageBytes();
}

QVariant PLCMessage::value(const QString& header, const QString& valueName)
{
    for (const auto& head : m_dataStructure)
    {
        if (head.first == header)
        {
            for (const auto& pair : head.second)
            {
                if (pair.first == valueName)
                {
                    return pair.second;
                }
            }
        }
    }
    return QVariant();
}

QVariant PLCMessage::value(int row, int column)
{
    if (row < m_dataStructure.length() && column < m_dataStructure.at(row).second.length())
    {
        return m_dataStructure.at(row).second.at(column).second;
    }
    return QVariant();
}

QVariant PLCMessage::value(int index)
{
    auto counter = 0;

    for (auto head : m_dataStructure)
    {
        for (auto pair : head.second)
        {
            if (index == counter)
            {
                return pair.second;
            }
            ++counter;
        }
    }

    return QVariant();
}

int PLCMessage::messageCounterIndex()
{
    if (m_messageCounterIndex >= 0)
    {
        return m_messageCounterIndex;
    }
    return -1;
}

QString PLCMessage::messageCounterTag()
{
    auto index = 0;
    for (auto header : m_dataStructure)
    {
        for (auto valuePair : header.second)
        {
            if (index == m_messageCounterIndex)
            {
                return valuePair.first;
            }
            ++index;
        }
    }
    return "";
}

void PLCMessage::setMessageCounter(const QString& messageCounterTag)
{
    auto index = 0;

    for (auto header : m_dataStructure)
    {
        for (auto valuePair : header.second)
        {
            if (valuePair.first == messageCounterTag)
            {
                m_messageCounterIndex = index;
                return;
            }
            ++index;
        }
    }
}

void PLCMessage::setValue(const QString& header, const QString& valueName, const QVariant& value)
{
    for (auto headerIndex = 0; headerIndex < m_dataStructure.length(); ++headerIndex)
    {
        auto& headerName = m_dataStructure.at(headerIndex).first;

        if (headerName == header)
        {
            auto& headerValues = m_dataStructure.at(headerIndex).second;

            for (auto valuePairIndex = 0; valuePairIndex < headerValues.length(); ++valuePairIndex)
            {
                auto valuePairName = headerValues.at(valuePairIndex).first;

                if (valuePairName == valueName)
                {
                    auto& oldValue = m_dataStructure[headerIndex].second[valuePairIndex].second;
                    if (oldValue.type() == value.type())
                    {
                        if (!m_differentialLogging && m_logger
                                && !m_filteredValues.contains(valueName))
                        {
                            m_logger->log(QString("%0 : %1 : %2 : %3 -> %4")
                                                  .arg(m_id,
                                                          headerName,
                                                          valueName,
                                                          oldValue.toString(),
                                                          value.toString()));
                        }
                        else if (oldValue != value && m_logger
                                && !m_filteredValues.contains(valueName))
                        {
                            m_logger->log(QString("%0 : %1 : %2 : %3 -> %4")
                                                  .arg(m_id,
                                                          headerName,
                                                          valueName,
                                                          oldValue.toString(),
                                                          value.toString()));
                        }
                        oldValue = value;
                        const auto valueId = headerName + SEPARATOR + valueName;
                    }
                    else
                    {
                        std::cout << "Cant assaing value:" << value.toInt()
                                  << "To :" << header.toStdString() << "->"
                                  << valueName.toStdString()
                                  << "Types dont match.\nExpected value:" << oldValue.typeName()
                                  << "Got: " << value.typeName() << "\n";
                    }
                }
            }
            break;
        }
    }
}

void PLCMessage::setValue(int index, const QVariant& value)
{
    auto counter = 0;

    for (auto header : m_dataStructure)
    {
        for (auto valuePair : header.second)
        {
            if (counter == index)
            {
               setValue(header.first, valuePair.first, value);
                return;
            }
            ++counter;
        }
    }
}

void PLCMessage::parseMessageBytes()
{
    QByteArray byteArray(messageSize(), 0);
    QVector<bool> bitCounter;
    auto index = 0;

    for (const auto& header : m_dataStructure)
    {
        for (const auto& pair : header.second)
        {
            auto fullName = header.first + SEPARATOR + pair.first;
            auto value = pair.second;
            auto type = static_cast<QMetaType::Type>(value.type());

            if (type == QMetaType::QByteArray)
            {
                const auto byte = quint8(value.toUInt());
                byteArray[index] = byte;
                ++index;
            }

            else if (type == QMetaType::Int)
            {
                const auto byte = qint16(value.toInt());

                quint8 lowBits = byte & 0x00ff;
                quint8 highBits = byte >> 8 & 0x00ff;

                byteArray[index] = highBits;
                ++index;

                byteArray[index] = lowBits;
                ++index;
            }
            else if (type == QMetaType::UInt)
            {
                const auto byte = quint16(value.toUInt());

                quint8 lowBits = byte & 0x00ff;
                quint8 highBits = byte >> 8 & 0x00ff;

                byteArray[index] = highBits;
                ++index;

                byteArray[index] = lowBits;
                ++index;
            }

            else if (type == QMetaType::Bool)
            {
                bitCounter.append(value.toBool());

                if (bitCounter.length() == 8)
                {
                    quint8 byte = 0;

                    for (int i = 0; i < 8; i++)
                    {
                        if (bitCounter.at(i) == true)
                            byte |= (1 << i);
                    }

                    byteArray[index] = byte;
                    bitCounter.clear();
                    ++index;
                }
            }

            else if (type == QMetaType::Float)
            {
                float floatValue = value.toFloat();
                quint32* int32Value = reinterpret_cast<quint32*>(&floatValue);
                quint8 firstByte = *int32Value >> 24 & 0x000000ff;
                quint8 secondByte = *int32Value >> 16 & 0x000000ff;
                quint8 thirdByte = *int32Value >> 8 & 0x000000ff;
                quint8 fourthByte = *int32Value & 0x000000ff;

                byteArray[index] = firstByte;
                ++index;
                byteArray[index] = secondByte;
                ++index;
                byteArray[index] = thirdByte;
                ++index;
                byteArray[index] = fourthByte;
                ++index;
            }

            else if (type == QMetaType::QChar)
            {
                const auto integer = char(value.toInt());
                byteArray[index] = integer;
                ++index;
            }
        }
    }
    m_bytes = byteArray;
 }

InternalData PLCMessage::dataStructure()
{
    return m_dataStructure;
}

QByteArray PLCMessage::bytes()
{
    return m_bytes;
}

void PLCMessage::setBytes(QByteArray bytes)
{
    if (bytes.length() != static_cast<int>(messageSize()))
    {
        std::cout << m_id.toStdString()
                        + " Received UDP Datagram which length does not match with the the DB "
                          "file: "
                        + m_dbPath.toStdString() + "\n";
        return;
    }

    auto byteIndex = 0;
    auto booleanIndex = 0;

    for (auto header : m_dataStructure)
    {
        for (auto valuePair : header.second)
        {
            auto type = static_cast<QMetaType::Type>(valuePair.second.type());
            auto valueTag = valuePair.first;
            auto headerTag = header.first;

            if (type == QMetaType::QByteArray)
            {
                QVariant value = QByteArray::number(quint8(bytes.at(byteIndex)));
                setValue(headerTag, valueTag, value);
                ++byteIndex;
            }

            else if (type == QMetaType::QChar)
            {
                QVariant value = QChar(bytes.at(byteIndex));
                setValue(headerTag, valueTag, value);
                ++byteIndex;
            }

            else if (type == QMetaType::Int)
            {
                quint8 highBits = bytes.at(byteIndex);
                quint8 lowBits = bytes.at(++byteIndex);
                qint16 value = highBits << 8 | lowBits;
                setValue(headerTag, valueTag, value);
                ++byteIndex;
            }

            else if (type == QMetaType::UInt)
            {
                quint8 highBits = bytes.at(byteIndex);
                quint8 lowBits = bytes.at(++byteIndex);
                uint value = highBits << 8 | lowBits;
                setValue(headerTag, valueTag, value);
                ++byteIndex;
            }

            else if (type == QMetaType::Float)
            {
                float value;
                char byts[] = {bytes.at(byteIndex + 3),
                        bytes.at(byteIndex + 2),
                        bytes.at(byteIndex + 1),
                        bytes.at(byteIndex)};
                memcpy(&value, &byts, sizeof(value));
                setValue(headerTag, valueTag, value);
                byteIndex += 4;
            }

            else if (type == QMetaType::Bool)
            {
                auto byte = bytes.at(byteIndex);
                auto currentBit = 0;
                while (currentBit < 8)
                {
                    if (byte & 0x01 && booleanIndex == currentBit)
                    {
                        setValue(headerTag, valueTag, true);
                    }
                    else if (booleanIndex == currentBit)
                    {
                        setValue(headerTag, valueTag, false);
                    }

                    ++currentBit;
                    byte = byte >> 1;
                }
                booleanIndex = booleanIndex + 1;

                if (booleanIndex == 8)
                {
                    booleanIndex = 0;
                    ++byteIndex;
                }
            }
        }
    }
    emit bytesReceived();
}

void PLCMessage::printDataStructureAsJson()
{
    std::cout << "{\n";

    for (auto header : m_dataStructure)
    {
        QString openingLine("   \"" + header.first + "\": {");
        std::cout << openingLine.toStdString() << "\n";

        for (auto pair : header.second)
        {
            auto name = pair.first;
            auto value = pair.second;
            auto type = static_cast<QMetaType::Type>(pair.second.type());

            QString valueString;

            if (type == QMetaType::Int || type == QMetaType::UInt)
            {
                valueString.append("      \"" + name + "\": " + value.toString());
            }

            else if (type == QMetaType::QChar)
            {
                QString tmpValue = "";
                if (!value.isNull())
                {
                    tmpValue = value.toString();
                }

                valueString.append("      \"" + name + "\": \"" + tmpValue + "\"");
            }

            else if (type == QMetaType::Float)
            {
                QString val = value.toInt() == 0 ? "0.0" : QString::number(value.toDouble());

                valueString.append("      \"" + name + "\": " + val);
            }

            else if (type == QMetaType::Bool)
            {
                valueString.append("      \"" + name + "\": " + value.toString());
            }
            else if (type == QMetaType::QByteArray)
            {
                valueString.append("      \"" + name + "\": " + QString::number(value.toInt()));
            }

            if (pair != header.second.back())
            {
                valueString.append(",");
            }

            std::cout << valueString.toStdString() << "\n";
        }

        QString closingLine = header == m_dataStructure.back() ? "   }\n" : "   },\n";
        std::cout << closingLine.toStdString();
    }
    std::cout << "}\n";
}

void PLCMessage::filterLogging(const QString& valueName)
{
    m_filteredValues.append(valueName);
}

QString PLCMessage::id() const
{
    return m_id;
}

void PLCMessage::criticalError(const QString& description)
{
    throw std::runtime_error(description.toStdString());
}

const QString PLCMessage::readFileStr(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
    {
        auto msg = QString("File not found: %0").arg(filename);
        criticalError(msg);
    }

    return file.readAll();
}

const QStringList PLCMessage::readFile(const QString& filename)
{
    auto str = readFileStr(filename);

    return str.split("\n");
}

uint PLCMessage::messageSize()
{
    float size = 0;

    for (const auto& header : m_dataStructure)
    {
        for (const auto& pair : header.second)
        {
            auto value = pair.second;
            auto type = static_cast<QMetaType::Type>(value.type());

            if (type == QMetaType::Int || type == QMetaType::UInt)
            {
                size = size + 2;
            }

            else if (type == QMetaType::QChar || type == QMetaType::QByteArray)
            {
                ++size;
            }

            else if (type == QMetaType::Float)
            {
                size = size + 4;
            }

            else if (type == QMetaType::Bool)
            {
                size = size + 0.125;
            }
        }
    }
    return qRound(size);
}

void visitStructure(QMap<QString, parser::TypeDefinition> const& types,
        QString const& path,
        parser::Structure const& structure,
        VariableTypeVisitor visitor);

void visitVariableTypeDefinition(QMap<QString, parser::TypeDefinition> const& types,
        QString const& path,
        parser::VariableTypeDefinition const& v,
        VariableTypeVisitor visitor)
{
    if (visitor(path, v))
        return;

    if (v.type == parser::VariableType::array)
    {
        parser::Array const& array = std::get<parser::Array>(v.structure);
        for (int i = array.low; i <= array.high; ++i)
            visitVariableTypeDefinition(types,
                    path + "[" + QString::number(i) + "]",
                    *std::get<parser::Array>(v.structure).variable,
                    visitor);
    }
    else if (v.type == parser::VariableType::structure)
    {
        visitStructure(types, path, std::get<parser::Structure>(v.structure), visitor);
    }
    else if (v.type == parser::VariableType::type)
    {
        visitor(path, v);

        parser::TypeReference const& type = std::get<parser::TypeReference>(v.structure);
        auto i = types.find(type.typeName);
        if (i == types.end())
            qWarning() << "Unable to find type " << type.typeName;
        else
            visitStructure(types, path, i->structure, visitor);
    }
}
void visitStructure(QMap<QString, parser::TypeDefinition> const& types,
        QString const& path,
        parser::Structure const& structure,
        VariableTypeVisitor visitor)
{
    for (parser::VariableDefinition const& v : structure.variables)
    {
        visitVariableTypeDefinition(types, path + "." + v.name, v.type, visitor);
    }
}
void visitDataBlock(QMap<QString, parser::TypeDefinition> const& types,
        parser::DataBlock const& dataBlock,
        VariableTypeVisitor visitor)
{
    visitStructure(types, "root", dataBlock.structure, visitor);
}
