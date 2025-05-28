#pragma once

#include <QSharedPointer>
#include <QJsonObject>
#include <QObject>

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
    void connectToDevice(const QUuid &uuid, const QHostAddress &host);

    void sendMessage(const QUuid &uuid, const QJsonObject &json);
    void sendRemoteControlMessage(const QUuid &master, const QUuid &slave);

    void handleRemoveDevice(const QUuid &uuid);
    void handleDeviceConnected(TcpSocket* socket, const QJsonObject &json);
    void handleDeviceDisconnected(TcpSocket* socket);

signals:
    void started();
    void finished();

    void deviceConnectionChanged(const QUuid &uuid, SharedCursor::ConnectionState state);
    void remoteControl(const QUuid &master, const QUuid &slave);
    void cursorPosition(const QPoint &pos);
    void cursorInitPosition(const QPoint &pos);
    void cursorDelta(const QPoint &pos);
    void keyboardEvent(int keycode, bool state);
    void mouseEvent(int button, bool state);
    void wheelEvent(int delta);
    void clipboard(const QUuid &uuid, const QJsonObject &json);

private slots:
    void onSocketConnected(qintptr socketDescriptor);
    void onMessageReceived(const QUuid &uuid, const QJsonObject &json);

private:
    QUuid _uuid;
    QString _keyword;
    QJsonObject _jsonRemoteControl;
    quint16 _port = SharedCursor::DEFAULT_TCP_PORT;
    QMap<QUuid, QSharedPointer<TcpSocket>> _devices;
    QVector<QSharedPointer<TcpSocket>> _tempSockets;
    QSharedPointer<TcpServer> _server;

    QJsonObject devicePtrToJsonObject(QSharedPointer<SharedCursor::Device> device);
    QSharedPointer<SharedCursor::Device> jsonObjectToDevicePtr(const QJsonObject &obj);

    QSharedPointer<TcpSocket> createSocket();
    void disconnectSocket(QSharedPointer<TcpSocket> socket);

    void pushTempSocket(QSharedPointer<TcpSocket> socket);
    QSharedPointer<TcpSocket> popTempSocket(TcpSocket* socket);
};
