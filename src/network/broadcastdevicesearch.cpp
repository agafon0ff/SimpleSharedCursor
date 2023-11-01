#include <QJsonObject>
#include <QDebug>

#include "broadcastdevicesearch.h"
#include "settingsloader.h"
#include "utils.h"

BroadcastDeviceSearch::BroadcastDeviceSearch(QObject *parent)
    : QObject{parent}
{
    connect(&udpSocket, &QUdpSocket::readyRead, this, &BroadcastDeviceSearch::onSocketReadyRead);

    sslWraper.setKey(Settings.value(KEY_KEYWORD).toString().toLocal8Bit());

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

    Utils::fillDeviceJsonMessage(jsonOut, KEY_SEARCH_REQUEST);
    Utils::convertJsonToArray(jsonOut, datagram);
    sslWraper.encrypt(datagram, datagramEnc);
    udpSocket.writeDatagram(datagramEnc, QHostAddress::Broadcast, port);
}

void BroadcastDeviceSearch::setPort(quint16 _port)
{
    qDebug() << Q_FUNC_INFO << _port;
    port = _port;
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
    QString type = jsonIn.value(KEY_TYPE).toString();
    jsonIn.insert(KEY_HOST, QHostAddress(host.toIPv4Address()).toString());

    if (type == KEY_SEARCH_REQUEST)
        handleSearchRequest(host, jsonIn);
    else if (type == KEY_SEARCH_RESPONSE)
        handleSearchResponse(jsonIn);

    Q_UNUSED(port);
}

void BroadcastDeviceSearch::handleSearchRequest(const QHostAddress &host, const QJsonObject &jObject)
{
    qDebug() << Q_FUNC_INFO;

    if (!jObject.contains(KEY_UUID) || !jObject.contains(KEY_NAME))
        return;

    if (Settings.value(KEY_UUID).toString() == jObject.value(KEY_UUID).toString())
        return;

    Utils::fillDeviceJsonMessage(jsonOut, KEY_SEARCH_RESPONSE);
    Utils::convertJsonToArray(jsonOut, datagram);

    sslWraper.encrypt(datagram, datagramEnc);
    udpSocket.writeDatagram(datagramEnc, host, port);
}

void BroadcastDeviceSearch::handleSearchResponse(const QJsonObject &jObject)
{
    qDebug() << Q_FUNC_INFO;

    if (jObject.contains(KEY_UUID) && jObject.contains(KEY_NAME))
        emit deviceFound(jObject);
}