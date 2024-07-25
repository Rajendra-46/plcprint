#include "udpserver.h"
#include "plcmessage.h"

#include <QThread>
UDPServer::UDPServer(const QString& ip, const QString& port, std::shared_ptr<PLCMessage> msg) :
    m_message(msg), m_socket(QUdpSocket()), m_ip(ip), m_port(port)
{
    connect(&m_socket,
            &QUdpSocket::readyRead,
            this,
            &UDPServer::readPendingDatagrams,
            Qt::DirectConnection);
    m_socket.bind(QHostAddress(m_ip), m_port.toInt());
}

bool UDPServer::recieving()
{
    return m_listening;
}

void UDPServer::startListening()
{
    m_listening = true;
}

void UDPServer::stopListening()
{
    m_listening = false;
}

void UDPServer::readPendingDatagrams()
{
    if (m_listening)
    {
        m_message->setBytes(m_socket.receiveDatagram().data());
    }
    else
    {
        m_socket.receiveDatagram();
    }
}
