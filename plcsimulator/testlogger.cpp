#include "testlogger.h"
#include <QDebug>

TestLogger::TestLogger(const QString& filename, bool timestamp, bool deleteOldLog) :
    m_timeStamp(timestamp), m_deleteOld(deleteOldLog), m_mutex(std::mutex())
{
    m_logFile.setFileName(filename);
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text))
        throw std::runtime_error(
                QString("Unable to open file %0 for logging.").arg(filename).toStdString());

    if (m_deleteOld)
    {
        m_logFile.resize(0);
    }
}

void TestLogger::log(const QString& msg)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    QTextStream stream(&m_logFile);

    if (m_timeStamp)
    {
        stream << timestamp(msg) << endl;
        return;
    }

    stream << msg << endl;
    return;
}

QString TestLogger::timestamp(const QString& msg)
{
    auto stampedMsg = QString("%0 :: %1").arg(QTime::currentTime().toString(m_timeStampFormat), msg);
    return stampedMsg;
}
