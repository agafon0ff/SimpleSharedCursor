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

void DeviceConnectManager::setUuid(const QUuid &_uuid)
{
    qDebug() << Q_FUNC_INFO << _uuid.toString();
    uuid = _uuid;
}

void DeviceConnectManager::setKeyword(const QString &_keyword)
{
    qDebug() << Q_FUNC_INFO;
    keyword = _keyword;

    auto i = devices.constBegin();
    while (i != devices.constEnd()) {
        i.value().socket->setKeyword(keyword);
        ++i;
    }
}

void DeviceConnectManager::saveDevices()
{
    qDebug() << Q_FUNC_INFO;

    QJsonObject jsonDevices;
    auto i = devices.constBegin();
    while (i != devices.constEnd()) {
        auto &device = i.value().device;
        jsonDevices.insert(device->uuid.toString(), devicePtrToJsonObject(device));
        ++i;
    }

    Settings.setValue(KEY_DEVICES, jsonDevices);
}

void DeviceConnectManager::loadDevices()
{
    qDebug() << Q_FUNC_INFO;

    const QJsonObject &jsonDevices = Settings.value(KEY_DEVICES).toObject();
    for (auto it=jsonDevices.constBegin(); it != jsonDevices.constEnd(); ++it) {
        QSharedPointer<Device> device = jsonObjectToDevicePtr(it.value().toObject());
        devices.insert(device->uuid, { device, QSharedPointer<TcpSocket>()});
        emit deviceChanged(device);
        connectToDevice(device);
    }
}

void DeviceConnectManager::start()
{
    qDebug() << Q_FUNC_INFO;

    devices.clear();
    loadDevices();

    server = QSharedPointer<TcpServer>(new TcpServer);
    server->setPort(port);
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

void DeviceConnectManager::handleDeviceFound(const QJsonObject &obj)
{
    QUuid uuid = QUuid::fromString(obj.value(KEY_UUID).toString());

    qDebug() << Q_FUNC_INFO << uuid;

    if (devices.contains(uuid))
        return;

    QSharedPointer<Device> device = jsonObjectToDevicePtr(obj);
    saveDevices();
    connectToDevice(device);
    emit deviceChanged(device);
}

void DeviceConnectManager::handleRemoveDevice(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO;
    if (!devices.contains(uuid))
        return;

    devices.remove(uuid);
    saveDevices();
}

void DeviceConnectManager::handleDeviceConnected(TcpSocket *socket, const QJsonObject &obj)
{
    qDebug() << Q_FUNC_INFO;

    QUuid uuid = QUuid::fromString(obj.value(KEY_UUID).toString());
    QSharedPointer<TcpSocket> socketPtr = popTempSocket(socket);

    if (socketPtr.isNull())
        return;

    QSharedPointer<Device> device;

    if (devices.contains(uuid)) {
        devices[uuid].socket.swap(socketPtr);
        device = devices[uuid].device;
    }
    else {
        device = jsonObjectToDevicePtr(obj);
        devices.insert(uuid, { device, socketPtr } );
    }

    device->state = Device::Connected;
    saveDevices();
    emit deviceChanged(device);
}

void DeviceConnectManager::handleDeviceDisconnected(TcpSocket *socket)
{
    if (!socket)
        return;

    QUuid uuid = socket->getUuid();

    qDebug() << Q_FUNC_INFO << uuid;

    if (devices.contains(uuid)) {
        QSharedPointer<TcpSocket> socket = devices.value(uuid).socket;
        disconnect(socket.get(), &TcpSocket::deviceConnected, this, &DeviceConnectManager::handleDeviceConnected);
        disconnect(socket.get(), &TcpSocket::deviceDisconnected, this, &DeviceConnectManager::handleDeviceDisconnected);

        QSharedPointer<Device> device = devices.value(uuid).device;
        device->state = Device::Disconnected;
        emit deviceChanged(device);
        devices[uuid].socket.clear();
    }
    else {
        popTempSocket(socket);
    }
}

void DeviceConnectManager::handleScreenPositionChanged(const QUuid &uuid, const QPoint &pos)
{
    qDebug() << Q_FUNC_INFO;
    if (!devices.contains(uuid))
        return;

    QSharedPointer<Device> device = devices.value(uuid).device;
    device->position = pos;
    saveDevices();
}

void DeviceConnectManager::onSocketConnected(qintptr socketDescriptor)
{
    QSharedPointer<TcpSocket> socket = createTempSocket();
    socket->setSocketDescriptor(socketDescriptor);
    socket->setType(TcpSocket::Type::ServerOwned);
}

void DeviceConnectManager::connectToDevice(QSharedPointer<Device> device)
{
    if (device.isNull() || device->self || device->state == Device::Connected)
        return;

    QSharedPointer<TcpSocket> socket = createTempSocket();
    socket->setType(TcpSocket::Type::Independent);
    socket->setUuid(device->uuid);
    socket->setHost(device->host);
    socket->setPort(port);
    socket->start();

    device->state = Device::Waiting;

    if (devices.contains(device->uuid)) {
        devices[device->uuid].socket.clear();
    }
}

QJsonObject DeviceConnectManager::devicePtrToJsonObject(QSharedPointer<Device> device)
{
    QJsonObject result;
    result.insert(KEY_UUID, device->uuid.toString());
    result.insert(KEY_NAME, device->name);
    result.insert(KEY_HOST, QHostAddress(device->host.toIPv4Address()).toString());
    result.insert(KEY_SELF, device->self);
    result.insert(KEY_SCREENS, Utils::rectListToJsonValue(device->screens));
    return result;
}

QSharedPointer<Device> DeviceConnectManager::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<Device> device = QSharedPointer<Device>(new Device);
    device->uuid = QUuid::fromString(obj.value(KEY_UUID).toString());
    device->name = obj.value(KEY_NAME).toString();
    device->host = QHostAddress(obj.value(KEY_HOST).toString());
    device->self = device->uuid == uuid;
    device->screens = Utils::jsonValueToRectList(obj.value(KEY_SCREENS));
    return device;
}

QSharedPointer<TcpSocket> DeviceConnectManager::createTempSocket()
{
    QSharedPointer<TcpSocket> socket = QSharedPointer<TcpSocket>(new TcpSocket);
    socket->setKeyword(keyword);
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
