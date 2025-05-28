#include <QJsonObject>
#include <QtEndian>
#include <QDebug>

#include "tcpsocket.h"
#include "utils.h"

static const int sizeofInt32 = 4;

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

    disconnect(this, &QTcpSocket::readyRead, this, &TcpSocket::onReadyRead);
    disconnect(this, &QTcpSocket::connected, this, &TcpSocket::onConnected);
    disconnect(this, &QTcpSocket::disconnected, this, &TcpSocket::onDisconnected);

    stop();
}

void TcpSocket::setType(Type type)
{
    qDebug() << Q_FUNC_INFO << type;
    _type = type;
}

TcpSocket::Type TcpSocket::getType() const
{
    return _type;
}

bool TcpSocket::isUuidEqual(const QUuid &_uuid) const
{
    return _uuid == _uuid;
}

QUuid TcpSocket::getUuid() const
{
    return _uuid;
}

void TcpSocket::setKeyword(const QString &keyword)
{
    _sslWraper.setKey(keyword.toLocal8Bit());
}

bool TcpSocket::isConnected() const
{
    return _isConnected;
}

void TcpSocket::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}

void TcpSocket::setHost(const QHostAddress &host)
{
    _host = host;
}

void TcpSocket::setPort(quint16 port)
{
    qDebug() << Q_FUNC_INFO << port;
    _port = port;
}

void TcpSocket::start()
{
    if (state() != QTcpSocket::UnconnectedState) stop();

    connectToHost(_host, _port);
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
    if (state() == QTcpSocket::ConnectedState) {
        SharedCursor::convertJsonToArray(json, _dataOut);
        _sslWraper.encrypt(_dataOut.constData(), _dataOut.size(), _dataOutEnc);
        appendDataSizeToOutBuffer();
        write(_dataOutEnc);
    }
}

void TcpSocket::appendDataSizeToOutBuffer()
{
    int size = _dataOutEnc.size();
    _dataOutEnc.resize(size + sizeofInt32);
    qToBigEndian(size, _dataOutEnc.data() + size);
}

void TcpSocket::extractDataSizesFromInputData()
{
    _dataSizes.clear();
    int size = _dataIn.size();

    for(int i=0; i<_dataIn.size(); ++i) {
        if (size > sizeofInt32) {
            int length = qFromBigEndian<quint32>(_dataIn.data() + (size - sizeofInt32));
            if (length + sizeofInt32 <= size) {
                _dataSizes.push(length);
                size -= length + sizeofInt32;
            }
            else {
                break;
            }
        }
    }
}

void TcpSocket::parseInputData(const QByteArray &data)
{
    if (!SharedCursor::convertArrayToJson(data, _jsonIn)) {
        qDebug() << Q_FUNC_INFO << "ERROR: Json parsing!" << _dataInDec;
        return;
    }

    if (_isConnected) {
        emit message(_uuid, _jsonIn);
    }
    else {
        if (!_jsonIn.contains(SharedCursor::KEY_UUID)) {
            return;
        }

        _jsonIn.insert(SharedCursor::KEY_HOST, QHostAddress(peerAddress().toIPv4Address()).toString());
        _messageType = _jsonIn.value(SharedCursor::KEY_TYPE).toString();

        if (_messageType == SharedCursor::KEY_CONNECT_REQUEST) {
            onConnectRequestReceived();
        }
        else if (_messageType == SharedCursor::KEY_CONNECT_RESPONSE) {
            if (!_isConnected) {
                _isConnected = true;
                emit deviceConnected(this, _jsonIn);
            }
        }
    }
}

void TcpSocket::onReadyRead()
{
    _dataIn.resize(bytesAvailable());
    read(_dataIn.data(), _dataIn.size());
    extractDataSizesFromInputData();

    int step = 0;
    while (!_dataSizes.isEmpty()) {
        int size = _dataSizes.pop();
        _sslWraper.decrypt(_dataIn.data() + step, size, _dataInDec);
        parseInputData(_dataInDec);
        step += size + sizeofInt32;
    }
}

void TcpSocket::onConnected()
{
    SharedCursor::fillDeviceJsonMessage(_jsonOut, SharedCursor::KEY_CONNECT_REQUEST);
    sendMessage(_jsonOut);
}

void TcpSocket::onDisconnected()
{
    if (_isConnected) {
        _isConnected = false;
        emit deviceDisconnected(this);
    }
}

void TcpSocket::onConnectRequestReceived()
{
    if (_uuid.isNull())
        _uuid = QUuid::fromString(_jsonIn.value(SharedCursor::KEY_UUID).toString());
    
    if (!_isConnected) {
        _isConnected = true;
        emit deviceConnected(this, _jsonIn);
    }

    SharedCursor::fillDeviceJsonMessage(_jsonOut, SharedCursor::KEY_CONNECT_RESPONSE);
    sendMessage(_jsonOut);
}
