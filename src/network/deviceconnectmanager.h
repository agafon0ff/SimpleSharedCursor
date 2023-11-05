#pragma once

#include <QObject>
#include <QSharedPointer>

#include "tcpsocket.h"
#include "tcpserver.h"
#include "global.h"

class DeviceConnectManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceConnectManager(QObject *parent = nullptr);
    ~DeviceConnectManager();

    void setPort(quint16 port);
    void setUuid(const QUuid &uuid);
    void setKeyword(const QString &keyword);

public slots:
    void start();
    void stop();

    void handleDeviceFound(const QJsonObject &obj);
    void handleRemoveDevice(const QUuid &uuid);
    void handleDeviceConnected(TcpSocket* socket, const QJsonObject &obj);
    void handleDeviceDisconnected(TcpSocket* socket);

signals:
    void started();
    void finished();

    void deviceConnectionChanged(const QUuid &uuid, Device::ConnectionState state);

private slots:
    void onSocketConnected(qintptr socketDescriptor);

private:
    QUuid _uuid;
    QString _keyword;
    quint16 _port = DEFAULT_TCP_PORT;
    QMap<QUuid, QSharedPointer<TcpSocket>> devices;
    QVector<QSharedPointer<TcpSocket>> tempSockets;
    QSharedPointer<TcpServer> server;

    void connectToDevice(const QUuid &uuid, const QString &host);
    QJsonObject devicePtrToJsonObject(QSharedPointer<Device> device);
    QSharedPointer<Device> jsonObjectToDevicePtr(const QJsonObject &obj);
    QSharedPointer<TcpSocket> createTempSocket();
    QSharedPointer<TcpSocket> popTempSocket(TcpSocket* socket);
};
