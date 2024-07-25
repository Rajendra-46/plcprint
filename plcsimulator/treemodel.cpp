#include "treemodel.h"
#include "plcmessage.h"
#include "QtMath"
#include <QDir>
#include <QFile>
#include <QCoreApplication>
TreeModel::TreeModel(std::shared_ptr<PLCMessage> msg,
                     ModelType type,
                     bool filterSpare,
                     bool fuzz,
                     uint fuzzInterval,
                     QObject* parent) :
    QAbstractItemModel(parent),
    m_fuzz(fuzz),
    m_filterSpare(filterSpare),
    m_type(type),
    m_message(msg)
{
    m_rootItem = new TreeItem("Name", "Value", false, 0, 0, 0, 0,nullptr);
    populateModel(m_message, m_rootItem);

    m_roleNameMapping[int(AccessRole::Name)] = "name";
    m_roleNameMapping[int(AccessRole::Value)] = "value";
    m_roleNameMapping[int(AccessRole::Type)] = "type";
    m_roleNameMapping[int(AccessRole::Fuzz)] = "fuzz";

    connect(m_message.get(), &PLCMessage::bytesReceived, this, &TreeModel::refreshView);

    SineWaveBytePoints();
    SineWaveIntPoints();
    SawtoothBytePoints();
    SawtoothIntPoints();
    TriangleWaveBytePoints();
    TriangleWaveIntPoints();

    if (m_type == ModelType::Sending)
    {
        connect(&m_fuzzClock, &QTimer::timeout, this, &TreeModel::fuzzify);
        m_fuzzClock.start(fuzzInterval);
    }

}

TreeModel::~TreeModel()
{
    delete m_rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return m_rootItem->data(static_cast<TreeItem::Type>(section));
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    if (!parentItem)
    {
        return QModelIndex();
    }

    TreeItem* childItem = parentItem->child(row);

    if (childItem)
    {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    if (!childItem)
    {
        return QModelIndex();
    }

    TreeItem* parentItem = childItem->parentItem();

    if (!parentItem || parentItem == m_rootItem)
    {
        return QModelIndex();
    }
    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    TreeItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    return m_rootItem->columnCount();
}

bool TreeModel::hasChildren(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        TreeItem* Item = static_cast<TreeItem*>(parent.internalPointer());
        return Item->childCount() > 0;
    }
    return false;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_message)
    {
        return QVariant();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    auto name = item->data(TreeItem::Type::Name);
    auto value = item->data(TreeItem::Type::Value);
    auto isHeader = item->children().length() != 0;
    auto type = value.type();
    auto msgCounter = m_message->messageCounterTag();

    if (role == int(AccessRole::Name))
    {
        return name;
    }
    else if (role == int(AccessRole::Value))
    {
        if (name == msgCounter && !msgCounter.isEmpty())
        {
            return "dynamic";
        }
        else if (type == QVariant::Char)
        {
            return value.toInt();
        }
        else if (type == QVariant::Int)
        {
            return value.toInt();
        }
        else if (type == QVariant::UInt)
        {
            return value.toUInt();
        }
        else if (type == QVariant::Bool)
        {
            return value;
        }
        else if (type == QVariant::ByteArray)
        {
            return value.toInt();
        }
        else if (static_cast<QMetaType::Type>(type) == QMetaType::Float)
        {
            return QString::number(value.toFloat(), 'G', 2);
        }
    }
    else if (role == int(AccessRole::Type))
    {
        if (type == QVariant::Char)
        {
            return QVariant("Char");
        }
        else if (type == QVariant::Int)
        {
            return QVariant("Int");
        }
        else if (type == QVariant::UInt)
        {
            return QVariant("UInt");
        }
        else if (type == QVariant::Bool)
        {
            return QVariant("Bool");
        }
        else if (type == QVariant::ByteArray)
        {
            return QVariant("Byte");
        }
        else if (static_cast<QMetaType::Type>(type) == QMetaType::Float)
        {
            return QVariant("Float");
        }
    }

    auto fuzz = item->data(TreeItem::Type::Fuzz);
    auto fuzzMin = item->data(TreeItem::Type::FuzzMin);
    auto fuzzMax = item->data(TreeItem::Type::FuzzMax);
    auto avg = item->data(TreeItem::Type::avg);
    auto selectedwave = item->data(TreeItem::Type::selectedwave);
    auto fuzzInterval = item->data(TreeItem::Type::FuzzInterval_s);

    if (role == int(AccessRole::Fuzz))
    {
        if (name == msgCounter)
        {
            return "";
        }
        else if (!isHeader)
        {
            return fuzz.toBool();
        }
    }
    else if (role == int(AccessRole::FuzzMin))
    {
        return fuzzMin;
    }

    else if (role == int(AccessRole::FuzzMax))
    {
        return fuzzMax;
    }
    else if (role == int(AccessRole::FuzzInterval_s))
    {
        return fuzzInterval;
    }
    else if (role == int(AccessRole::avg))
    {
        return avg;
    }
    else if (role == int(AccessRole::selectedwave))
    {
        return selectedwave;
    }
    return QVariant("");
}

bool TreeModel::writeData(const QModelIndex& index, const QVariant& value, const QString& role)
{
    if (!index.isValid() || !m_message)
    {
        return false;
    }

    auto item = static_cast<TreeItem*>(index.internalPointer());
    auto oldValue = m_message->value(item->parentItem()->data(TreeItem::Type::Name).toString(),
                                     item->data(TreeItem::Type::Name).toString());
    auto type = oldValue.type();

    auto min = minValue(oldValue);
    auto max = maxValue(oldValue);
    auto avg = avgValue(oldValue);
    auto wave = waveValue(oldValue);
    if (role == "value")
    {
        auto newValue = processInput(value, type, min, max , avg, wave);

        if (item && newValue.has_value())
        {
            item->setData(TreeItem::Type::Value, newValue.value());
            m_message->setValue(item->parentItem()->data(TreeItem::Type::Name).toString(),
                                item->data(TreeItem::Type::Name).toString(),
                                newValue.value());
        }
        emit dataChanged(index, index);
        return newValue.has_value();
    }

    else if (role == "fuzz")
    {
        if (item)
        {
            item->setData(TreeItem::Type::Fuzz, value);
            emit dataChanged(index, index);
        }
        return true;
    }

    else if (role == "fuzzMin")
    {
        auto newValue = processInput(value, type, min, max, avg, wave);
        auto max = item->data(TreeItem::Type::FuzzMax).toInt();
        if (item && newValue.has_value())
        {
            if (newValue.value().toInt() > max)
            {
                return false;
            }
            item->setData(TreeItem::Type::FuzzMin, newValue.value());
        }
        return newValue.has_value();
    }

    else if (role == "fuzzMax")
    {
        auto newValue = processInput(value, type, min, max, avg, wave);
        auto min = item->data(TreeItem::Type::FuzzMin).toInt();

        if (item && newValue.has_value())
        {
            if (newValue.value().toInt() < min)
            {
                return false;
            }
            item->setData(TreeItem::Type::FuzzMax, newValue.value());
        }
        return newValue.has_value();
    }

    else if (role == "avg")
    {
        auto newValue = processInput(value, type, min, max, avg, wave);
        auto avg = item->data(TreeItem::Type::avg).toInt();
        item->setData(TreeItem::Type::avg, newValue.value());
        return newValue.has_value();
    }
    else if (role == "waves")
    {
        auto newValue = processInput(value, type, min, max, avg, wave);
        auto wave = item->data(TreeItem::Type::selectedwave).toInt();
        item->setData(TreeItem::Type::selectedwave, newValue.value());
        return newValue.has_value();
    }

    else if (role == "fuzzInterval")
    {
        if (item)
        {
            bool success = false;
            auto interval = value.toInt(&success);

            if (success)
            {
                item->setData(TreeItem::Type::FuzzInterval_s, interval);
            }
            return success;
        }
    }

    return false;
}

std::optional<QVariant> TreeModel::processInput(const QVariant& input,
                                                QVariant::Type type,
                                                int min,
                                                int max,
                                                int avg,
                                                int wave)
{
    bool success = true;
    auto comparator = input.toInt(&success);
    if (type == QVariant::Char)
    {
        if (input.toString().isEmpty())
        {
            return std::nullopt;
        }
        else if (!input.toString().at(0).isNumber())
        {
            return QVariant(QChar(input.toString().at(0)));
        }

        if (success && comparator >= min && comparator <= max)
        {
            return QVariant(QChar(input.toInt()));
        }

        else
        {
            return std::nullopt;
        }
    }

    else if (type == QVariant::Int)
    {
        if (success && comparator >= min && comparator <= max)
        {
            return QVariant(input.toInt());
        }
        else
        {
            return std::nullopt;
        }
    }

    else if (type == QVariant::UInt)
    {
        if (success && comparator >= min && comparator <= max)
        {
            return QVariant(input.toUInt());
        }
        else
        {
            return std::nullopt;
        }
    }

    else if (type == QVariant::ByteArray)
    {
        if (success && comparator >= min && comparator <= max)
        {
            return QVariant(input.toByteArray());
        }
        else
        {
            return std::nullopt;
        }
    }

    else if (type == QVariant::Bool)
    {
        return QVariant(input.toBool());
    }

    else if (static_cast<QMetaType::Type>(type) == QMetaType::Float)
    {
        return QVariant(input.toFloat());
    }

    else
    {
        return std::nullopt;
    }
}

QVariant TreeModel::readData(const QModelIndex& index, const QString& role)
{
    AccessRole accessRole = AccessRole::Value;
    if (role == "name")
    {
        accessRole = AccessRole::Name;
    }
    else if (role == "type")
    {
        accessRole = AccessRole::Type;
    }
    else if (role == "value")
    {
        accessRole = AccessRole::Value;
    }
    else if (role == "fuzz")
    {
        accessRole = AccessRole::Fuzz;
    }
    else if (role == "fuzzMin")
    {
        accessRole = AccessRole::FuzzMin;
    }
    else if (role == "fuzzMax")
    {
        accessRole = AccessRole::FuzzMax;
    }
    else if (role == "avg")
    {
        accessRole = AccessRole::avg;
    }
    else if (role == "selectedwave")
    {
        accessRole = AccessRole::selectedwave;
    }
    else if (role == "fuzzInterval")
    {
        accessRole = AccessRole::FuzzInterval_s;
    }
    return data(index, int(accessRole));
}

QString TreeModel::modelType() const
{
    switch (m_type)
    {
    case ModelType::Sending:
        return "sending";
    case ModelType::Receiving:
        return "receiving";
    }
    return "none";
}

QString TreeModel::modelId() const
{
    if (m_message)
    {
        return m_message->id();
    }
    return "";
}

QVariant TreeModel::type(const QModelIndex& index) const
{
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item)
    {
        return QString(item->data(TreeItem::Type::Value).type());
    }
    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QHash<int, QByteArray> TreeModel::roleNames() const
{
    return m_roleNameMapping;
}

void TreeModel::refreshView()
{
    if (!m_rootItem)
    {
        return;
    }

    for (const auto& parent : m_rootItem->children())
    {
        for (const auto& item : parent->children())
        {
            auto parentName = parent->data(TreeItem::Type::Name);
            auto itemName = item->data(TreeItem::Type::Name);

            if (m_filterSpare && m_spareRE.match(itemName.toString()).hasMatch())
            {
                continue;
            }

            auto value = m_message->value(parentName.toString(), itemName.toString());
            item->setData(TreeItem::Type::Value, value);
            auto index = createIndex(item->row(), 0, item);
            emit dataChanged(index, index);
        }
    }
}

void TreeModel::fuzzify()
{
    if (!m_message || !m_rootItem)
    {
        return;
    }
    noItr_Int = (noItr_Int+1) % 100;
    noItr_Byte = (noItr_Byte+1) % 100;
    int num;
    for (const auto item : m_rootItem->children())
    {
        for (const auto child : item->children())
        {
            auto lastFuzz_ms = child->timeElapsedSinceFuzz_ms();

            auto fuzzInterval_ms = child->data(TreeItem::Type::FuzzInterval_s).toInt() * 1000;

            if (child->data(TreeItem::Type::Fuzz).toBool() && lastFuzz_ms > fuzzInterval_ms)
            {
                auto parentName = item->data(TreeItem::Type::Name).toString();
                auto name = child->data(TreeItem::Type::Name).toString();
                auto type = child->data(TreeItem::Type::Value).type();
                auto min = child->data(TreeItem::Type::FuzzMin).toInt();
                auto avg = child->data(TreeItem::Type::avg).toInt();
                auto waves = child->data(TreeItem::Type::selectedwave).toInt();
                auto max = child->data(TreeItem::Type::FuzzMax).toInt() + 1; // +1 = [0-1[ -> [0-1]

                if (name == m_message->messageCounterTag())
                {
                    continue;
                }

                if (type == QVariant::Char)
                {
                    const auto randomChar = QRandomGenerator::global()->bounded(min, max);
                    child->setData(TreeItem::Type::Value, QVariant(randomChar).toChar());
                    m_message->setValue(parentName, name, QVariant(randomChar).toChar());
                }
                else if (type == QVariant::Int)
                {
                    if(waves == 10)
                    {
                        QString noLine = strlst_Int_points.at(noItr_Int);
                        num = noLine.toInt();
                        child->setData(TreeItem::Type::Value, QVariant(num));
                        m_message->setValue(parentName, name, QVariant(num));
                    }else if(waves == 11)
                    {
                        QString noLine = strlstSawth_Int_points.at(noItr_Int);
                        num = noLine.toInt();
                        child->setData(TreeItem::Type::Value, QVariant(num));
                        m_message->setValue(parentName, name, QVariant(num));

                    }else if(waves == 12)
                    {
                        QString noLine = strlstDecay_Int_points.at(noItr_Int);
                        num = noLine.toInt();
                        child->setData(TreeItem::Type::Value, QVariant(num));
                        m_message->setValue(parentName, name, QVariant(num));
                    }else
                    {
                        const int randomInt = QRandomGenerator::global()->bounded(min, max);
                        child->setData(TreeItem::Type::Value, QVariant(randomInt));
                        m_message->setValue(parentName, name, QVariant(randomInt));
                    }

                }
                else if (type == QVariant::UInt)
                {
                    const uint randomUInt = QRandomGenerator::global()->bounded(min, max);
                    child->setData(TreeItem::Type::Value, QVariant(randomUInt));
                    m_message->setValue(parentName, name, QVariant(randomUInt));
                }

                else if (type == QVariant::ByteArray)
                {
                    if(waves == 10)
                    {
                        QString noLine = strlst_Byte_points.at(noItr_Byte);
                        num = noLine.toInt();
                        QByteArray q_bAry;
                        child->setData(TreeItem::Type::Value, QVariant(q_bAry.setNum(num)));
                        m_message->setValue(parentName, name, QVariant(q_bAry.setNum(num)));
                    }else if(waves == 11)
                    {
                        QString noLine = strlstSawth_Byte_points.at(noItr_Byte);
                        num = noLine.toInt();
                        QByteArray q_bAry;
                        child->setData(TreeItem::Type::Value, QVariant(q_bAry.setNum(num)));
                        m_message->setValue(parentName, name, QVariant(q_bAry.setNum(num)));

                    }else if(waves == 12)
                    {
                        QString noLine = strlstDecay_Byte_points.at(noItr_Byte);
                        num = noLine.toInt();
                        QByteArray q_bAry;
                        child->setData(TreeItem::Type::Value, QVariant(q_bAry.setNum(num)));
                        m_message->setValue(parentName, name, QVariant(q_bAry.setNum(num)));

                    }else
                    {
                        const auto randomByte = QRandomGenerator::global()->bounded(min, max);
                        child->setData(TreeItem::Type::Value, QVariant(randomByte).toByteArray());
                        m_message->setValue(parentName, name, QVariant(randomByte).toByteArray());
                    }


                }

                else if (type == QVariant::Bool)
                {
                    const auto randomBool = bool(QRandomGenerator::global()->bounded(0, 2));
                    child->setData(TreeItem::Type::Value, QVariant(randomBool).toBool());
                    m_message->setValue(parentName, name, QVariant(randomBool).toBool());
                }

                else if (static_cast<QMetaType::Type>(type) == QMetaType::Float)
                {
                    const auto randomFloat = QRandomGenerator::global()->generateDouble();
                    child->setData(TreeItem::Type::Value, QVariant(randomFloat).toFloat());
                    m_message->setValue(parentName, name, QVariant(randomFloat).toFloat());
                }

                child->resetFuzzTimer();
                emit dataChanged(  createIndex(child->row(), 0, child), createIndex(child->row(), 1, child));
            }
        }
    }
    m_message->parseMessageBytes();
}
qint64 TreeModel::minValue(const QVariant& value)
{
    auto type = value.type();

    if (type == QVariant::Char)
    {
        return std::numeric_limits<char>::min();
    }
    else if (type == QVariant::Int)
    {
        return std::numeric_limits<qint16>::min();
    }
    else if (type == QVariant::UInt)
    {
        return std::numeric_limits<quint16>::min();
    }
    else if (type == QVariant::ByteArray)
    {
        return std::numeric_limits<quint8>::min();
    }
    return 0;
}

qint64 TreeModel::maxValue(const QVariant& value)
{
    auto type = value.type();

    if (type == QVariant::Char)
    {
        return std::numeric_limits<char>::max();
    }
    else if (type == QVariant::Int)
    {
        return std::numeric_limits<qint16>::max();
    }
    else if (type == QVariant::UInt)
    {
        return std::numeric_limits<quint16>::max();
    }
    else if (type == QVariant::ByteArray)
    {
        return std::numeric_limits<quint8>::max();
    }
    return 0;
}
qint64 TreeModel::avgValue(const QVariant& value)
{
    auto type = value.type();

    if (type == QVariant::Char)
    {
        return std::numeric_limits<char>::max();
    }
    else if (type == QVariant::Int)
    {
        return std::numeric_limits<qint16>::max();
    }
    else if (type == QVariant::UInt)
    {
        return std::numeric_limits<quint16>::max();
    }
    else if (type == QVariant::ByteArray)
    {
        return std::numeric_limits<quint8>::max();
    }
    return 0;
}
qint64 TreeModel::waveValue(const QVariant& value)
{
    auto type = value.type();
    if (type == QVariant::Char)
    {
        return std::numeric_limits<char>::max();
    }
    else if (type == QVariant::Int)
    {
        return std::numeric_limits<qint16>::max();
    }
    else if (type == QVariant::UInt)
    {
        return std::numeric_limits<quint16>::max();
    }
    else if (type == QVariant::ByteArray)
    {
        return std::numeric_limits<quint8>::max();
    }
    return 0;
}
void TreeModel::populateModel(std::shared_ptr<PLCMessage> msg, TreeItem* parent)
{
    auto plcmsg = msg->dataStructure();

    for (auto header : plcmsg)
    {
        TreeItem* item = new TreeItem(header.first, QVariant(""), false, 0, 0, 0, 0,parent);
        for (auto pair : header.second)
        {
            if (m_filterSpare && m_spareRE.match(pair.first).hasMatch())
            {
                continue;
            }
            else
            {
                TreeItem* child = new TreeItem(pair.first,
                                               pair.second,
                                               m_fuzz,
                                               minValue(pair.second),
                                               maxValue(pair.second),
                                               avgValue(pair.second),
                                               waveValue(pair.second),
                                               item);

                item->appendChild(child);
            }
        }
        m_rootItem->appendChild(item);
    }
}
void TreeModel::SineWaveBytePoints()
{
    QFile file(qApp->applicationDirPath()+"/SineWaveByte.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"ERROR !! Could't not open the wavepoints.txt";
    }
    else
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            strlst_Byte_points << line;
        }
    }
    file.close();
}
void TreeModel::SineWaveIntPoints()
{
    QFile file(qApp->applicationDirPath()+"/SineWaveInt.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"ERROR !! Could't not open the wavepoints.txt";
    }
    else
    {

        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            strlst_Int_points << line;
        }
    }
    file.close();
}

void TreeModel::SawtoothBytePoints()
{
    QFile file(qApp->applicationDirPath()+"/SawtoothWaveByte.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"ERROR !! Could't not open the wavepoints.txt";
    }
    else
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            strlstSawth_Byte_points << line;
        }
    }
    file.close();

}

void TreeModel::SawtoothIntPoints()
{
    QFile file(qApp->applicationDirPath()+"/SawtoothWaveInt.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"ERROR !! Could't not open the wavepoints.txt";
    }
    else
    {

        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            strlstSawth_Int_points << line;
        }
    }
    file.close();
}

void TreeModel::TriangleWaveBytePoints()
{
    QFile file(qApp->applicationDirPath()+"/TriangleWaveByte.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"ERROR !! Could't not open the wavepoints.txt";
    }
    else
    {

        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            strlstDecay_Byte_points << line;
        }
    }
    file.close();
}

void TreeModel::TriangleWaveIntPoints()
{
    QFile file(qApp->applicationDirPath()+"/TriangleWaveInt.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"ERROR !! Could't not open the wavepoints.txt";
    }
    else
    {

        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            strlstDecay_Int_points << line;
        }
    }
    file.close();
}
