#include "workerthread.h"
#include "udpclient.h"
#include "udpserver.h"
#include <QDebug>

WorkerThread::WorkerThread(const QString& videoServerIp,
        const QString& videoServerPort,
        std::shared_ptr<PLCMessage> msgSend,
        int sendIntervarl,
        const QString& hostedIp,
        const QString& hostedPort,
        std::shared_ptr<PLCMessage> msgReceive) :
    m_thread(QThread())
{
    m_server = std::make_unique<UDPServer>(hostedIp, hostedPort, msgReceive);
    m_client = std::make_unique<UDPClient>(videoServerIp, videoServerPort, sendIntervarl, msgSend);
    m_server->moveToThread(this);
    m_client->moveToThread(this);
    start();
}

WorkerThread::~WorkerThread()
{
    stopSending();
    stopReceiving();
    quit();
    wait();
}

void WorkerThread::startSending()
{
    m_client->startSending();
    emit statusChanged();
}

void WorkerThread::stopSending()
{
    m_client->stopSending();
    emit statusChanged();
}

void WorkerThread::startReceiving()
{
    m_server->startListening();
    emit statusChanged();
}

void WorkerThread::stopReceiving()
{
    m_server->stopListening();
    emit statusChanged();
}

bool WorkerThread::receiving()
{
    return m_server->recieving();
}

bool WorkerThread::sending()
{
    return m_client->sending();
}
