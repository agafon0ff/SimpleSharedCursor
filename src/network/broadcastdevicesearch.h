#pragma once

#include <QSharedPointer>
#include <QJsonObject>
#include <QUdpSocket>

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

signals:
    void deviceFound(const QJsonObject& jObject);

private:
    quint16 port = DEFAULT_UDP_PORT;
    QUdpSocket udpSocket;
    OpenSslWrapper sslWraper;
    QByteArray datagram, datagramEnc;
    QJsonObject jsonIn, jsonOut;

private slots:
    void onSocketReadyRead();
    void onNewData(const QHostAddress &host, quint16 port, const QByteArray &data);
    void handleSearchRequest(const QHostAddress &host, const QJsonObject& jObject);
    void handleSearchResponse(const QJsonObject& jObject);
};
