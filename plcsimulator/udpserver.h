#include <QHostAddress>
#include <QNetworkDatagram>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

#include <memory>

class PLCMessage;
class UDPServer : public QObject
{
    Q_OBJECT
public:
    UDPServer(const QString& ip, const QString& port, std::shared_ptr<PLCMessage> msg);

public slots:
    void startListening();
    void stopListening();
    void readPendingDatagrams();
    bool recieving();

private:
    std::shared_ptr<PLCMessage> m_message;
    QUdpSocket m_socket;
    QString m_ip;
    QString m_port;
    bool m_listening = false;
};
