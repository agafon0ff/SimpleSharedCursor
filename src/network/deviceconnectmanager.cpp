#include <QJsonArray>

#include "deviceconnectmanager.h"
#include "settingsfacade.h"
#include "utils.h"

DeviceConnectManager::DeviceConnectManager(QObject *parent)
    : QObject{parent}
{
    qDebug() << Q_FUNC_INFO;
}

DeviceConnectManager::~DeviceConnectManager()
{
    qDebug() << Q_FUNC_INFO;
    tempSockets.clear();
    devices.clear();
}

void DeviceConnectManager::setPort(quint16 port)
{
    _port = port;
}

void DeviceConnectManager::setUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid.toString();
    _uuid = uuid;
}

void DeviceConnectManager::setKeyword(const QString &keyword)
{
    qDebug() << Q_FUNC_INFO;
    _keyword = keyword;

    auto i = devices.constBegin();
    while (i != devices.constEnd()) {
        i.value()->setKeyword(_keyword);
        ++i;
    }
}

void DeviceConnectManager::start()
{
    qDebug() << Q_FUNC_INFO;

    devices.clear();

    server = QSharedPointer<TcpServer>(new TcpServer);
    server->setPort(_port);
    server->start();

    connect(server.get(), &TcpServer::newSocketConnected, this, &DeviceConnectManager::onSocketConnected);

    emit started();
}

void DeviceConnectManager::stop()
{
    qDebug() << Q_FUNC_INFO;

    disconnect(server.get(), &TcpServer::newSocketConnected, this, &DeviceConnectManager::onSocketConnected);

    devices.clear();
    server.clear();

    emit finished();
}

void DeviceConnectManager::connectToDevice(const QUuid &uuid, const QHostAddress &host)
{
    qDebug() << Q_FUNC_INFO << uuid << host;

    if (uuid.isNull() || uuid == _uuid)
        return;

    if (devices.contains(uuid)) {
        QSharedPointer<TcpSocket> socket = devices.value(uuid);
        if(!socket.isNull() && socket->isConnected()) {
            return;
        }
    }

    QSharedPointer<TcpSocket> socket = createTempSocket();
    socket->setType(TcpSocket::Type::Independent);
    socket->setUuid(uuid);
    socket->setHost(host);
    socket->setPort(_port);
    socket->start();

    if (devices.contains(uuid)) {
        devices[uuid].clear();
    }

    emit deviceConnectionChanged(uuid, ShareCursor::Waiting);
}

void DeviceConnectManager::handleRemoveDevice(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO;
    if (!devices.contains(uuid))
        return;

    devices.remove(uuid);
}

void DeviceConnectManager::handleDeviceConnected(TcpSocket *socket, const QJsonObject &obj)
{
    qDebug() << Q_FUNC_INFO;

    QUuid uuid = QUuid::fromString(obj.value(ShareCursor::KEY_UUID).toString());
    QSharedPointer<TcpSocket> socketPtr = popTempSocket(socket);

    if (socketPtr.isNull())
        return;

    if (devices.contains(uuid)) {
        devices[uuid].swap(socketPtr);
    }
    else {
        devices.insert(uuid, socketPtr);
    }

    emit deviceConnectionChanged(uuid, ShareCursor::Connected);
}

void DeviceConnectManager::handleDeviceDisconnected(TcpSocket *socket)
{
    if (!socket)
        return;

    QUuid uuid = socket->getUuid();

    qDebug() << Q_FUNC_INFO << uuid;

    if (devices.contains(uuid)) {
        QSharedPointer<TcpSocket> socket = devices.value(uuid);
        disconnect(socket.get(), &TcpSocket::deviceConnected, this, &DeviceConnectManager::handleDeviceConnected);
        disconnect(socket.get(), &TcpSocket::deviceDisconnected, this, &DeviceConnectManager::handleDeviceDisconnected);

        emit deviceConnectionChanged(uuid, ShareCursor::Disconnected);
        devices[uuid].clear();
    }
    else {
        popTempSocket(socket);
    }
}

void DeviceConnectManager::onSocketConnected(qintptr socketDescriptor)
{
    QSharedPointer<TcpSocket> socket = createTempSocket();
    socket->setSocketDescriptor(socketDescriptor);
    socket->setType(TcpSocket::Type::ServerOwned);
}

QJsonObject DeviceConnectManager::devicePtrToJsonObject(QSharedPointer<ShareCursor::Device> device)
{
    QJsonObject result;
    result.insert(ShareCursor::KEY_UUID, device->uuid.toString());
    result.insert(ShareCursor::KEY_NAME, device->name);
    result.insert(ShareCursor::KEY_HOST, QHostAddress(device->host.toIPv4Address()).toString());
    result.insert(ShareCursor::KEY_SELF, device->self);
    result.insert(ShareCursor::KEY_SCREENS, Utils::rectListToJsonValue(device->screens));
    return result;
}

QSharedPointer<ShareCursor::Device> DeviceConnectManager::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<ShareCursor::Device> device = QSharedPointer<ShareCursor::Device>(new ShareCursor::Device);
    device->uuid = QUuid::fromString(obj.value(ShareCursor::KEY_UUID).toString());
    device->name = obj.value(ShareCursor::KEY_NAME).toString();
    device->host = QHostAddress(obj.value(ShareCursor::KEY_HOST).toString());
    device->self = device->uuid == _uuid;
    device->screens = Utils::jsonValueToRectList(obj.value(ShareCursor::KEY_SCREENS));
    return device;
}

QSharedPointer<TcpSocket> DeviceConnectManager::createTempSocket()
{
    QSharedPointer<TcpSocket> socket = QSharedPointer<TcpSocket>(new TcpSocket);
    socket->setKeyword(_keyword);
    tempSockets.append(socket);

    connect(socket.get(), &TcpSocket::deviceConnected, this, &DeviceConnectManager::handleDeviceConnected, Qt::QueuedConnection);
    connect(socket.get(), &TcpSocket::deviceDisconnected, this, &DeviceConnectManager::handleDeviceDisconnected, Qt::QueuedConnection);

    return socket;
}

QSharedPointer<TcpSocket> DeviceConnectManager::popTempSocket(TcpSocket *socket)
{
    QSharedPointer<TcpSocket> result;
    for (auto &socketPtr: qAsConst(tempSockets)) {
        if (socketPtr.get() == socket) {
            result = socketPtr;
            tempSockets.removeOne(socketPtr);
            break;
        }
    }
    return result;
}
