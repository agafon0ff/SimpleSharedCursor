#pragma once

#include <QSharedPointer>
#include <QJsonObject>
#include <QUdpSocket>
#include <QUuid>

#include "opensslwrapper.h"
#include "global.h"

class BroadcastDeviceSearch : public QObject
{
    Q_OBJECT
public:
    explicit BroadcastDeviceSearch(QObject *parent = nullptr);
    ~BroadcastDeviceSearch();

public slots:
    void start();
    void stop();
    void search();
    void setPort(quint16 port);
    void setUuid(const QUuid &_uuid);
    void setKeyword(const QString &keyword);

signals:
    void deviceFound(const QJsonObject& jObject);

private:
    quint16 _port = SharedCursor::DEFAULT_UDP_PORT;
    QString _uuid;
    QUdpSocket _udpSocket;
    OpenSslWrapper _sslWraper;
    QByteArray _datagram, _datagramEnc;
    QJsonObject _jsonIn, _jsonOut;

private slots:
    void onSocketReadyRead();
    void onNewData(const QHostAddress &host, quint16 port, const QByteArray &data);
    void handleSearchRequest(const QHostAddress &host, const QJsonObject& jObject);
    void handleSearchResponse(const QJsonObject& jObject);
};
