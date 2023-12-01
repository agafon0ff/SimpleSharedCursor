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
    void connectToDevice(const QUuid &uuid, const QHostAddress &host);

    void sendMessage(const QUuid &uuid, const QJsonObject &json);

    void handleRemoveDevice(const QUuid &uuid);
    void handleDeviceConnected(TcpSocket* socket, const QJsonObject &json);
    void handleDeviceDisconnected(TcpSocket* socket);

signals:
    void started();
    void finished();

    void deviceConnectionChanged(const QUuid &uuid, ShareCursor::ConnectionState state);
    void controlledByUuid(const QUuid &uuid);
    void remoteCursorPosition(const QUuid &uuid, const QPoint &pos);

    void cursorPosition(const QPoint &pos);
    void cursorDelta(const QPoint &pos);
    void keyboardEvent(int keycode, bool state);
    void mouseEvent(int button, bool state);
    void wheelEvent(int delta);

private slots:
    void onSocketConnected(qintptr socketDescriptor);
    void onMessageReceived(const QUuid &uuid, const QJsonObject &json);

private:
    QUuid _uuid;
    QString _keyword;
    QJsonObject jsonMessage;
    quint16 _port = ShareCursor::DEFAULT_TCP_PORT;
    QMap<QUuid, QSharedPointer<TcpSocket>> devices;
    QVector<QSharedPointer<TcpSocket>> tempSockets;
    QSharedPointer<TcpServer> server;

    QJsonObject devicePtrToJsonObject(QSharedPointer<ShareCursor::Device> device);
    QSharedPointer<ShareCursor::Device> jsonObjectToDevicePtr(const QJsonObject &obj);
    QSharedPointer<TcpSocket> createTempSocket();
    QSharedPointer<TcpSocket> popTempSocket(TcpSocket* socket);
};
