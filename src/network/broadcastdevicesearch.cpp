#include <QJsonObject>
#include <QDebug>

#include "broadcastdevicesearch.h"
#include "settingsfacade.h"
#include "utils.h"

BroadcastDeviceSearch::BroadcastDeviceSearch(QObject *parent)
    : QObject{parent}
{
    connect(&udpSocket, &QUdpSocket::readyRead, this, &BroadcastDeviceSearch::onSocketReadyRead);
    qDebug() << Q_FUNC_INFO;
}

BroadcastDeviceSearch::~BroadcastDeviceSearch()
{
    qDebug() << Q_FUNC_INFO;
    stop();
}

void BroadcastDeviceSearch::start()
{
    qDebug() << Q_FUNC_INFO << port;

    if (udpSocket.state() == QAbstractSocket::BoundState)
        udpSocket.close();

    udpSocket.bind(QHostAddress::Any, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}

void BroadcastDeviceSearch::stop()
{
    qDebug() << Q_FUNC_INFO;

    if (udpSocket.state() == QAbstractSocket::BoundState)
        udpSocket.close();
}

void BroadcastDeviceSearch::search()
{
    qDebug() << Q_FUNC_INFO;

    Utils::fillDeviceJsonMessage(jsonOut, ShareCursor::KEY_SEARCH_REQUEST);
    Utils::convertJsonToArray(jsonOut, datagram);
    sslWraper.encrypt(datagram, datagramEnc);
    udpSocket.writeDatagram(datagramEnc, QHostAddress::Broadcast, port);
}

void BroadcastDeviceSearch::setPort(quint16 _port)
{
    qDebug() << Q_FUNC_INFO << _port;
    port = _port;
}

void BroadcastDeviceSearch::setUuid(const QUuid &_uuid)
{
    qDebug() << Q_FUNC_INFO << _uuid.toString();
    uuid = _uuid.toString();
}

void BroadcastDeviceSearch::setKeyword(const QString &keyword)
{
    qDebug() << Q_FUNC_INFO;
    sslWraper.setKey(keyword.toLocal8Bit());
}

void BroadcastDeviceSearch::onSocketReadyRead()
{
    QHostAddress senderHost;
    quint16 senderPort;

    while (udpSocket.hasPendingDatagrams()) {
        datagramEnc.resize(udpSocket.pendingDatagramSize());
        udpSocket.readDatagram(datagramEnc.data(), datagramEnc.size(),
                               &senderHost, &senderPort);

        onNewData(senderHost, senderPort, datagramEnc);
    }
}

void BroadcastDeviceSearch::onNewData(const QHostAddress &host, quint16 port, const QByteArray &data)
{
    sslWraper.decrypt(data, datagram);
    if (!Utils::convertArrayToJson(datagram, jsonIn)) return;

    qDebug() << Q_FUNC_INFO << host << port << jsonIn;
    QString type = jsonIn.value(ShareCursor::KEY_TYPE).toString();
    jsonIn.insert(ShareCursor::KEY_HOST, QHostAddress(host.toIPv4Address()).toString());

    if (type == ShareCursor::KEY_SEARCH_REQUEST)
        handleSearchRequest(host, jsonIn);
    else if (type == ShareCursor::KEY_SEARCH_RESPONSE)
        handleSearchResponse(jsonIn);

    Q_UNUSED(port);
}

void BroadcastDeviceSearch::handleSearchRequest(const QHostAddress &host, const QJsonObject &jObject)
{
    qDebug() << Q_FUNC_INFO;

    if (!jObject.contains(ShareCursor::KEY_UUID) ||
        !jObject.contains(ShareCursor::KEY_NAME))
        return;

    if (uuid == jObject.value(ShareCursor::KEY_UUID).toString())
        return;

    Utils::fillDeviceJsonMessage(jsonOut, ShareCursor::KEY_SEARCH_RESPONSE);
    Utils::convertJsonToArray(jsonOut, datagram);

    sslWraper.encrypt(datagram, datagramEnc);
    udpSocket.writeDatagram(datagramEnc, host, port);
    emit deviceFound(jObject);
}

void BroadcastDeviceSearch::handleSearchResponse(const QJsonObject &jObject)
{
    qDebug() << Q_FUNC_INFO;

    if (jObject.contains(ShareCursor::KEY_UUID) &&
        jObject.contains(ShareCursor::KEY_NAME))
        emit deviceFound(jObject);
}
