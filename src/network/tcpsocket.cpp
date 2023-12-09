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
    if (state() == QTcpSocket::ConnectedState) {
        SharedCursor::convertJsonToArray(json, dataOut);
        sslWraper.encrypt(dataOut, dataOutEnc);
        appendDataSizeToOutBuffer();
        write(dataOutEnc);
    }
}

void TcpSocket::appendDataSizeToOutBuffer()
{
    int size = dataOutEnc.size();
    dataOutEnc.resize(size + sizeofInt32);
    qToBigEndian(size, dataOutEnc.data() + size);
}

void TcpSocket::extractDataSizesFromInputData()
{
    dataSizes.clear();
    int size = dataIn.size();

    for(int i=0; i<dataIn.size(); ++i) {
        if (size > sizeofInt32) {
            int length = qFromBigEndian<quint32>(dataIn.data() + (size - sizeofInt32));
            if (length + sizeofInt32 <= size) {
                dataSizes.push(length);
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
    if (!SharedCursor::convertArrayToJson(data, jsonIn)) {
        qDebug() << Q_FUNC_INFO << "ERROR: Json parsing!" << dataInDec;
        return;
    }

    if (_isConnected) {
        emit message(uuid, jsonIn);
    }
    else {
        if (!jsonIn.contains(SharedCursor::KEY_UUID)) {
            return;
        }

        jsonIn.insert(SharedCursor::KEY_HOST, QHostAddress(peerAddress().toIPv4Address()).toString());
        messageType = jsonIn.value(SharedCursor::KEY_TYPE).toString();

        if (messageType == SharedCursor::KEY_CONNECT_REQUEST) {
            _isConnected = true;
            onConnectRequestReceived();
        }
        else if (messageType == SharedCursor::KEY_CONNECT_RESPONSE) {
            _isConnected = true;
            emit deviceConnected(this, jsonIn);
        }
    }
}

void TcpSocket::onReadyRead()
{
    dataIn.resize(bytesAvailable());
    read(dataIn.data(), dataIn.size());
    extractDataSizesFromInputData();

    int step = 0;
    while (!dataSizes.isEmpty()) {
        int size = dataSizes.pop();
        sslWraper.decrypt(dataIn.data() + step, size, dataInDec);
        parseInputData(dataInDec);
        step += size + sizeofInt32;
    }
}

void TcpSocket::onConnected()
{
    SharedCursor::fillDeviceJsonMessage(jsonOut, SharedCursor::KEY_CONNECT_REQUEST);
    sendMessage(jsonOut);
}

void TcpSocket::onDisconnected()
{
    _isConnected = false;
    emit deviceDisconnected(this);
}

void TcpSocket::onConnectRequestReceived()
{
    if (uuid.isNull())
        uuid = QUuid::fromString(jsonIn.value(SharedCursor::KEY_UUID).toString());

    emit deviceConnected(this, jsonIn);

    SharedCursor::fillDeviceJsonMessage(jsonOut, SharedCursor::KEY_CONNECT_RESPONSE);
    sendMessage(jsonOut);
}
