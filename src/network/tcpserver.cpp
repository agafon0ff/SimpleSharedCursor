#include <QSslSocket>
#include <QDebug>

#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent)
    : QTcpServer{parent}
{
    qDebug() << Q_FUNC_INFO;
}

TcpServer::~TcpServer()
{
    stop();
}

void TcpServer::setPort(quint16 port)
{
    qDebug() << Q_FUNC_INFO << port;
    _port = port;
}

void TcpServer::start()
{
    qDebug() << Q_FUNC_INFO;

    if (!listen(QHostAddress::Any, _port)) {
        qDebug() << Q_FUNC_INFO << "Error: Unable to start TCP server";
    }
}

void TcpServer::stop()
{
    qDebug() << Q_FUNC_INFO;

    if (isListening()) {
        close();
    }
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << Q_FUNC_INFO;

    emit newSocketConnected(socketDescriptor);
}
