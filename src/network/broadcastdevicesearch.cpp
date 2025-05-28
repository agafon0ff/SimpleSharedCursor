#include <QJsonObject>
#include <QDebug>

#include "broadcastdevicesearch.h"
#include "utils.h"

BroadcastDeviceSearch::BroadcastDeviceSearch(QObject *parent)
    : QObject{parent}
{
    connect(&_udpSocket, &QUdpSocket::readyRead, this, &BroadcastDeviceSearch::onSocketReadyRead);
    qDebug() << Q_FUNC_INFO;
}

BroadcastDeviceSearch::~BroadcastDeviceSearch()
{
    qDebug() << Q_FUNC_INFO;
    stop();
}

void BroadcastDeviceSearch::start()
{
    qDebug() << Q_FUNC_INFO << _port;

    if (_udpSocket.state() == QAbstractSocket::BoundState)
        _udpSocket.close();

    _udpSocket.bind(QHostAddress::Any, _port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}

void BroadcastDeviceSearch::stop()
{
    qDebug() << Q_FUNC_INFO;

    if (_udpSocket.state() == QAbstractSocket::BoundState)
        _udpSocket.close();
}

void BroadcastDeviceSearch::search()
{
    qDebug() << Q_FUNC_INFO;

    SharedCursor::fillDeviceJsonMessage(_jsonOut, SharedCursor::KEY_SEARCH_REQUEST);
    SharedCursor::convertJsonToArray(_jsonOut, _datagram);
    _sslWraper.encrypt(_datagram.constData(), _datagram.size(), _datagramEnc);
    _udpSocket.writeDatagram(_datagramEnc, QHostAddress::Broadcast, _port);
}

void BroadcastDeviceSearch::setPort(quint16 port)
{
    qDebug() << Q_FUNC_INFO << port;
    _port = port;
}

void BroadcastDeviceSearch::setUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid.toString();
    _uuid = uuid.toString();
}

void BroadcastDeviceSearch::setKeyword(const QString &keyword)
{
    qDebug() << Q_FUNC_INFO;
    _sslWraper.setKey(keyword.toLocal8Bit());
}

void BroadcastDeviceSearch::onSocketReadyRead()
{
    QHostAddress senderHost;
    quint16 senderPort;

    while (_udpSocket.hasPendingDatagrams()) {
        _datagramEnc.resize(_udpSocket.pendingDatagramSize());
        _udpSocket.readDatagram(_datagramEnc.data(), _datagramEnc.size(),
                               &senderHost, &senderPort);

        onNewData(senderHost, senderPort, _datagramEnc);
    }
}

void BroadcastDeviceSearch::onNewData(const QHostAddress &host, quint16 port, const QByteArray &data)
{
    _sslWraper.decrypt(data.constData(), data.size(), _datagram);
    if (!SharedCursor::convertArrayToJson(_datagram, _jsonIn)) return;

    qDebug() << Q_FUNC_INFO << host << port << _jsonIn;
    QString type = _jsonIn.value(SharedCursor::KEY_TYPE).toString();
    _jsonIn.insert(SharedCursor::KEY_HOST, QHostAddress(host.toIPv4Address()).toString());

    if (type == SharedCursor::KEY_SEARCH_REQUEST)
        handleSearchRequest(host, _jsonIn);
    else if (type == SharedCursor::KEY_SEARCH_RESPONSE)
        handleSearchResponse(_jsonIn);

    Q_UNUSED(port);
}

void BroadcastDeviceSearch::handleSearchRequest(const QHostAddress &host, const QJsonObject &jObject)
{
    qDebug() << Q_FUNC_INFO;

    if (!jObject.contains(SharedCursor::KEY_UUID) ||
        !jObject.contains(SharedCursor::KEY_NAME))
        return;

    if (_uuid == jObject.value(SharedCursor::KEY_UUID).toString())
        return;

    SharedCursor::fillDeviceJsonMessage(_jsonOut, SharedCursor::KEY_SEARCH_RESPONSE);
    SharedCursor::convertJsonToArray(_jsonOut, _datagram);

    _sslWraper.encrypt(_datagram.constData(), _datagram.size(), _datagramEnc);
    _udpSocket.writeDatagram(_datagramEnc, host, _port);
    emit deviceFound(jObject);
}

void BroadcastDeviceSearch::handleSearchResponse(const QJsonObject &jObject)
{
    qDebug() << Q_FUNC_INFO;

    if (jObject.contains(SharedCursor::KEY_UUID) &&
        jObject.contains(SharedCursor::KEY_NAME))
        emit deviceFound(jObject);
}
