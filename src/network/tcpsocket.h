#pragma once

#include <QSharedPointer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QUuid>

#include "opensslwrapper.h"
#include "global.h"

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpSocket(QObject *parent = nullptr);
    virtual ~TcpSocket();

    enum Type {
        Independent,
        ServerOwned
    };

    void setType(TcpSocket::Type _type);
    TcpSocket::Type getType() const;

    bool isUuidEqual(const QUuid &uuid) const;
    QUuid getUuid() const;

    friend bool operator==(const QUuid& uuid, const TcpSocket& socket) {
        return uuid == socket.uuid;
    }

signals:
    void deviceConnected(TcpSocket* self, const QJsonObject &json);
    void deviceDisconnected(TcpSocket* self);
    void message(const QJsonObject &json);

public slots:
    void setUuid(const QUuid &uuid);
    void setHost(const QHostAddress &host);
    void setPort(quint16 port);

    void start();
    void stop();

    void sendMessage(const QJsonObject &json);

private:
    QUuid uuid;
    QHostAddress host;
    quint16 port = DEFAULT_TCP_PORT;
    bool isConnected = false;
    QByteArray dataIn, dataInDec;
    QByteArray dataOut, dataOutEnc;
    QJsonObject jsonIn, jsonOut;
    QString messageType;
    OpenSslWrapper sslWraper;
    TcpSocket::Type type = TcpSocket::Type::Independent;

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onConnectRequestReceived();
};

inline QDebug operator<< (QDebug d, const TcpSocket::Type &type) {
    d << (type == TcpSocket::Type::Independent ? "Independent" : "ServerOwned");
    return d;
}
