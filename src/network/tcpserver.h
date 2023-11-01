#pragma once

#include <QSslCertificate>
#include <QSslSocket>
#include <QSslKey>
#include <QTcpServer>
#include <QTcpSocket>

#include "global.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    virtual ~TcpServer();

public slots:
    void setPort(quint16 port);
    void start();
    void stop();

signals:
    void newSocketConnected(qintptr socketDescriptor);

private:
    quint16 port = DEFAULT_TCP_PORT;
    void incomingConnection(qintptr socketDescriptor);

};
