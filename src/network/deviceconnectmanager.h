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

    void setUuid(const QUuid &_uuid);
    void setKeyword(const QString &_keyword);

    void saveDevices();
    void loadDevices();

public slots:
    void start();
    void stop();

    void handleDeviceFound(const QJsonObject &obj);
    void handleRemoveDevice(const QUuid &uuid);
    void handleDeviceConnected(TcpSocket* socket, const QJsonObject &obj);
    void handleDeviceDisconnected(TcpSocket* socket);
    void handleScreenPositionChanged(const QUuid &uuid, const QPoint &pos);

signals:
    void started();
    void finished();
    void deviceChanged(QSharedPointer<Device> device);

private slots:
    void onSocketConnected(qintptr socketDescriptor);

private:
    struct DevicePair {
        QSharedPointer<Device> device;
        QSharedPointer<TcpSocket> socket;
    };

    QUuid uuid;
    QString keyword;
    quint16 port = DEFAULT_TCP_PORT;
    QMap<QUuid, DevicePair> devices;
    QVector<QSharedPointer<TcpSocket>> tempSockets;
    QSharedPointer<TcpServer> server;

    void connectToDevice(QSharedPointer<Device> device);
    QJsonObject devicePtrToJsonObject(QSharedPointer<Device> device);
    QSharedPointer<Device> jsonObjectToDevicePtr(const QJsonObject &obj);
    QSharedPointer<TcpSocket> createTempSocket();
    QSharedPointer<TcpSocket> popTempSocket(TcpSocket* socket);
};
