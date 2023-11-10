#pragma once

#include <QTcpServer>
#include "global.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    virtual ~TcpServer();

    void setPort(quint16 port);

public slots:
    void start();
    void stop();

signals:
    void newSocketConnected(qintptr socketDescriptor);

private:
    quint16 port = ShareCursor::DEFAULT_TCP_PORT;
    void incomingConnection(qintptr socketDescriptor);

};
