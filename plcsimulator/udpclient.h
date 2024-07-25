#include <QHostAddress>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>
#include <qdebug.h>
#include <memory>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtCore>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>

class PLCMessage;
class UDPClient : public QObject
{
    Q_OBJECT

public:
    UDPClient(const QString& ip, const QString& port, int interval, std::shared_ptr<PLCMessage> msg);
    QWidget *groupWidget;
    QLabel *lbl_byteCount,*lbl_byteCountVal;
    QLabel *lbl_interval,*lbl_intervalVal,*lbl_ip,*lbl_port;
    QComboBox *cmb_bx;
    QLineEdit *line_editip,*line_editport;
    QString ipAddress;
    QString ipport;


public slots:
    Q_INVOKABLE void startSending();
    Q_INVOKABLE void stopSending();
    Q_INVOKABLE void sendDatagram();
    Q_INVOKABLE bool sending();

private:
    QUdpSocket m_socketOut;
    QTimer m_timer;
    std::shared_ptr<PLCMessage> m_message;
    QString m_ip;
    QString m_port;
    int m_interval;
    int Bytes = 0;
    int count;
    bool m_sending = false;
};
