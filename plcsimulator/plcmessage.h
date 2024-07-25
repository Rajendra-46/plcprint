#include "dbparser.h"
#include "testlogger.h"

#include <iostream>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPair>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QVariant>

typedef QVector<QPair<QString, QVector<QPair<QString, QVariant>>>> InternalData;
typedef QPair<QString, QVector<QPair<QString, QVariant>>> Entry;
const static QString SEPARATOR = "[SPERATOR]";

class PLCMessage : public QObject
{
    Q_OBJECT
public:
    PLCMessage(const QString& dbPath, const QString& id);
    ~PLCMessage(){};

    void enableLogging(std::shared_ptr<TestLogger> logger, bool diff = true);
    void enableConfigServer(const QString& address);
    void addUserSettings(const QString& jsonPath);

    InternalData dataStructure();

    QByteArray bytes();
    void setBytes(QByteArray bytes);

    QVariant value(const QString& header, const QString& valueName);
    QVariant value(int row, int column);
    QVariant value(int index);

    void setMessageCounter(const QString& messageCounterTag);

    int messageCounterIndex();
    QString messageCounterTag();

    void setValue(const QString& header, const QString& valueName, const QVariant& value);
    void setValue(int index, const QVariant& value);
    void printDataStructureAsJson();
    void filterLogging(const QString& valueName);
    QString id() const;
    void parseMessageBytes();

public slots:
    void processHTTPMessage();

signals:
    void bytesReceived();

private:
    void criticalError(const QString& description);
    void parseDataBlock(const QString& filename);
    const QStringList readFile(const QString& filename);
    const QString readFileStr(const QString& filename);
    uint messageSize();

    const QString m_id;
    const QString m_dbPath;
    const QString m_userSettingsJson;

    std::shared_ptr<TestLogger> m_logger = nullptr;
    bool m_differentialLogging;

    QByteArray m_bytes;
    int m_messageCounterIndex = -1;

    std::shared_ptr<QTcpServer> m_configurationServer;
    QStringList m_filteredValues;
    InternalData m_dataStructure;
};
