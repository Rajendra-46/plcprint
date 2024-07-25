#include "udpclient.h"
#include "plcmessage.h"
#include <QApplication>

UDPClient::UDPClient(const QString& targetIP,
        const QString& targetPort,
        int interval,
        std::shared_ptr<PLCMessage> msg) :
    m_socketOut(QUdpSocket()),
    m_timer(QTimer()),
    m_message(msg),
    m_ip(targetIP),
    m_port(targetPort),
    m_interval(interval)
{

    groupWidget = new QWidget();
    groupWidget->setGeometry(250,250,250,150);
    groupWidget->setFixedSize(350,280);
    groupWidget->setWindowTitle("Number of Packets sent...");

    lbl_ip = new QLabel("IP Address");
    line_editip = new QLineEdit();
    line_editip->setText("192.162.0.5");
    lbl_ip->setFixedSize(130,30);
    line_editip->setFixedSize(130,30);

    lbl_port = new QLabel("Port");
    line_editport = new QLineEdit();
    line_editport->setText("40001");
    lbl_port->setFixedSize(130,30);
    line_editport->setFixedSize(130,30);


    lbl_interval = new QLabel("Time Interval/ms");
    cmb_bx = new QComboBox();
    QStringList strlst;
    strlst << "10" << "20" << "30" <<"40" << "50" << "60" << "70" << "80" << "90" << "100" << "200" << "300" << "400" << "500" <<"1000";
    cmb_bx->addItems(strlst);
    cmb_bx->setCurrentText(strlst.at(4));
    lbl_interval->setFixedSize(130,30);
    cmb_bx->setFixedSize(130,30);
    cmb_bx->setStyleSheet("QLabel {background-color: grey;}");
   // cmb_bx->setText(QString::number(interval)+"ms");

    lbl_byteCount = new QLabel("Number of Packets");
    lbl_byteCountVal = new QLabel(" ");
    lbl_byteCount->setFixedSize(130,30);
    lbl_byteCountVal->setFixedSize(130,30);
    lbl_byteCountVal->setStyleSheet("QLabel {background-color: grey;}");

    QHBoxLayout *Hip = new QHBoxLayout();
    Hip->addWidget(lbl_ip);
    Hip->addWidget(line_editip);

    QHBoxLayout *Hport = new QHBoxLayout();
    Hport->addWidget(lbl_port);
    Hport->addWidget(line_editport);

    QHBoxLayout *H0Layout = new QHBoxLayout();
    H0Layout->addWidget(lbl_interval);
    H0Layout->addWidget(cmb_bx);

    QHBoxLayout *HLayout = new QHBoxLayout();
    HLayout->addWidget(lbl_byteCount);
    HLayout->addWidget(lbl_byteCountVal);

    QVBoxLayout *VLayout = new QVBoxLayout();
    VLayout->addLayout(Hip);
    VLayout->addLayout(Hport);
    VLayout->addLayout(H0Layout);
    VLayout->addLayout(HLayout);

    groupWidget->setLayout(VLayout);

   ipAddress = line_editip->text();
   ipport = line_editport->text();

   m_socketOut.connectToHost(ipAddress,ipport.toInt());
   // m_socketOut.connectToHost(QHostAddress::Any, qint16(m_port.toInt()));
   // m_socketOut.connectToHost(QHostAddress::Broadcast,40002);
    connect(&m_timer, &QTimer::timeout, this, &UDPClient::sendDatagram, Qt::DirectConnection);
}

bool UDPClient::sending()
{
    return m_sending;
}

void UDPClient::startSending()
{
    m_socketOut.close();
    ipAddress = line_editip->text();
    ipport = line_editport->text();
    m_sending = true;
    cmb_bx->setEnabled(false);
    groupWidget->show();
    int interval = cmb_bx->currentText().toInt();
    groupWidget->activateWindow();
    m_socketOut.connectToHost(ipAddress,ipport.toInt());
    qDebug() << "ipAddresstart===" << ipAddress << "ipportend==" << ipport;
    m_timer.start(interval);
}

void UDPClient::stopSending()
{
    cmb_bx->setEnabled(true);
    m_sending = false;
    Bytes = 0;
    groupWidget->activateWindow();
}

void UDPClient::sendDatagram()
{
    if (!m_sending)
    {
        return;
    }

    m_socketOut.write(m_message->bytes());
    QByteArray bytes = m_message->bytes();
    Bytes = Bytes + bytes.size();
    count = Bytes / 1110;
    lbl_byteCountVal->setText(QString::number(count));
   // qDebug() << (quint8)bytes.at(0) ;
   // qDebug() << (qint8)bytes.at(0) ;
    auto counterIndex = m_message->messageCounterIndex();

    if (counterIndex > -1)
    {
        auto msgCounter = m_message->value(counterIndex).toUInt();
        msgCounter = msgCounter + 1 <= 255 ? msgCounter + 1 : 0;
        m_message->setValue(counterIndex, QByteArray().setNum(msgCounter));
    }
}
