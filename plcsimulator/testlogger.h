#include <QFile>
#include <QTextStream>
#include <QTime>

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
class TestLogger
{
public:
    TestLogger(const QString& filename, bool timestamp, bool clearOldLog);
    void log(const QString& msg);
    QString timestamp(const QString& msg);

private:
    bool m_timeStamp = false;
    bool m_deleteOld = false;
    QFile m_logFile;
    QString m_timeStampFormat = "hh:mm:ss:zzzzzz";
    std::mutex m_mutex;
};
