#include <QJsonObject>
#include <QDebug>

#include "tcpsocket.h"
#include "settingsfacade.h"
#include "utils.h"

TcpSocket::TcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, &QTcpSocket::readyRead, this, &TcpSocket::onReadyRead);
    connect(this, &QTcpSocket::connected, this, &TcpSocket::onConnected);
    connect(this, &QTcpSocket::disconnected, this, &TcpSocket::onDisconnected);
}

TcpSocket::~TcpSocket()
{
    qDebug() << Q_FUNC_INFO;
    stop();
}

void TcpSocket::setType(Type _type)
{
    qDebug() << Q_FUNC_INFO << _type;
    type = _type;
}

TcpSocket::Type TcpSocket::getType() const
{
    return type;
}

bool TcpSocket::isUuidEqual(const QUuid &_uuid) const
{
    return uuid == _uuid;
}

QUuid TcpSocket::getUuid() const
{
    return uuid;
}

void TcpSocket::setKeyword(const QString &keyword)
{
    sslWraper.setKey(keyword.toLocal8Bit());
}

bool TcpSocket::isConnected() const
{
    return _isConnected;
}

void TcpSocket::setUuid(const QUuid &_uuid)
{
    uuid = _uuid;
}

void TcpSocket::setHost(const QHostAddress &_host)
{
    host = _host;
}

void TcpSocket::setPort(quint16 _port)
{
    qDebug() << Q_FUNC_INFO << _port;
    port = _port;
}

void TcpSocket::start()
{
    if (state() != QTcpSocket::UnconnectedState) stop();

    connectToHost(host, port);
}

void TcpSocket::stop()
{
    qDebug() << Q_FUNC_INFO;

    _isConnected = false;

    if (state() == QTcpSocket::ConnectedState)
        disconnectFromHost();
    else if (state() == QTcpSocket::ConnectingState)
        abort();
}

void TcpSocket::sendMessage(const QJsonObject &json)
{
    Utils::convertJsonToArray(json, dataOut);
    sslWraper.encrypt(dataOut, dataOutEnc);
    write(dataOutEnc);
}

void TcpSocket::onReadyRead()
{
    dataIn.resize(bytesAvailable());
    read(dataIn.data(), dataIn.size());
    sslWraper.decrypt(dataIn, dataInDec);

    if (!Utils::convertArrayToJson(dataInDec, jsonIn)) return;
    if (!jsonIn.contains(ShareCursor::KEY_UUID)) return;

    messageType = jsonIn.value(ShareCursor::KEY_TYPE).toString();

    if (_isConnected) {
        emit message(jsonIn);
    }
    else {
        jsonIn.insert(ShareCursor::KEY_HOST, QHostAddress(peerAddress().toIPv4Address()).toString());

        if (messageType == ShareCursor::KEY_CONNECT_REQUEST) {
            _isConnected = true;
            onConnectRequestReceived();
        }
        else if (messageType == ShareCursor::KEY_CONNECT_RESPONSE) {
            _isConnected = true;
            emit deviceConnected(this, jsonIn);
        }
    }
}

void TcpSocket::onConnected()
{
    Utils::fillDeviceJsonMessage(jsonOut, ShareCursor::KEY_CONNECT_REQUEST);
    Utils::convertJsonToArray(jsonOut, dataOut);
    sslWraper.encrypt(dataOut, dataOutEnc);
    write(dataOutEnc);
}

void TcpSocket::onDisconnected()
{
    _isConnected = false;
    emit deviceDisconnected(this);
}

void TcpSocket::onConnectRequestReceived()
{
    if (uuid.isNull())
        uuid = QUuid::fromString(jsonIn.value(ShareCursor::KEY_UUID).toString());

    emit deviceConnected(this, jsonIn);

    Utils::fillDeviceJsonMessage(jsonOut, ShareCursor::KEY_CONNECT_RESPONSE);
    sendMessage(jsonOut);
}
