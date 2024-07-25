#include <QObject>
#include <QThread>
#include <QTimer>

#include <memory>

class UDPClient;
class UDPServer;
class PLCMessage;

typedef std::unique_ptr<UDPClient> Client;
typedef std::unique_ptr<UDPServer> Server;

class WorkerThread : public QThread
{
    Q_OBJECT

public:
    WorkerThread(const QString& videoServerIp,
            const QString& videoServerPort,
            std::shared_ptr<PLCMessage> msgSend,
            int sendIntervarl,
            const QString& hostedIp,
            const QString& hostedPort,
            std::shared_ptr<PLCMessage> msgReceive);

    ~WorkerThread();
    Q_INVOKABLE bool receiving();
    Q_INVOKABLE bool sending();
    Q_INVOKABLE void startSending();
    Q_INVOKABLE void stopSending();
    Q_INVOKABLE void startReceiving();
    Q_INVOKABLE void stopReceiving();

signals:
    void statusChanged();

private:
    Client m_client;
    Server m_server;
    QThread m_thread;
};
