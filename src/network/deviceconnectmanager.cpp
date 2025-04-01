#include <QJsonArray>

#include "deviceconnectmanager.h"
#include "utils.h"

DeviceConnectManager::DeviceConnectManager(QObject *parent)
    : QObject{parent}
{
    qDebug() << Q_FUNC_INFO;
    jsonRemoteControl[SharedCursor::KEY_TYPE] = SharedCursor::KEY_REMOTE_CONTROL;
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
    qDebug() << Q_FUNC_INFO << uuid;
    _uuid = uuid;
}

void DeviceConnectManager::setKeyword(const QString &keyword)
{
    qDebug() << Q_FUNC_INFO;
    _keyword = keyword;

    for (auto it = devices.constBegin(); it != devices.constEnd(); ++it) {
        if (!it.value().isNull()) {
            it.value()->setKeyword(_keyword);
        }
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

    if (uuid.isNull() || uuid == _uuid) {
        return;
    }

    auto it = devices.find(uuid);
    if (it != devices.end()) {
        QSharedPointer<TcpSocket> socket = devices.value(uuid);
        if(!socket.isNull() && socket->isConnected()) {
            emit deviceConnectionChanged(uuid, SharedCursor::Connected);
            return;
        } else {
            it.value().clear();
        }
    }

    QSharedPointer<TcpSocket> socket = createSocket();
    socket->setType(TcpSocket::Type::Independent);
    socket->setUuid(uuid);
    socket->setHost(host);
    socket->setPort(_port);
    socket->start();
    pushTempSocket(socket);

    emit deviceConnectionChanged(uuid, SharedCursor::Waiting);
}

void DeviceConnectManager::sendMessage(const QUuid &uuid, const QJsonObject &json)
{
    auto it = devices.find(uuid);
    if (it != devices.end() && !it.value().isNull()) {
        it.value()->sendMessage(json);
    }
}

void DeviceConnectManager::sendRemoteControlMessage(const QUuid &master, const QUuid &slave)
{
    jsonRemoteControl[SharedCursor::KEY_MASTER] = master.toString();
    jsonRemoteControl[SharedCursor::KEY_SLAVE] = slave.toString();

    for (auto it = devices.constBegin(); it != devices.constEnd(); ++it) {
        if (it.key() != _uuid && !it.value().isNull()) {
            it.value()->sendMessage(jsonRemoteControl);
        }
    }
}

void DeviceConnectManager::handleRemoveDevice(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO;
    if (!devices.contains(uuid))
        return;

    devices.remove(uuid);
}

void DeviceConnectManager::handleDeviceConnected(TcpSocket *socket, const QJsonObject &json)
{
    QUuid uuid = QUuid::fromString(json.value(SharedCursor::KEY_UUID).toString());
    qDebug() << Q_FUNC_INFO << uuid;

    QSharedPointer<TcpSocket> socketPtr = popTempSocket(socket);

    if (socketPtr.isNull())
        return;

    auto it = devices.find(uuid);
    if (it != devices.end()) {
        if (!it.value()->isConnected()) {
            it.value().swap(socketPtr);
            emit deviceConnectionChanged(uuid, SharedCursor::Connected);
        }
        else {
            disconnectSocket(socketPtr);
        }
    } else {
        devices.insert(uuid, socketPtr);
        emit deviceConnectionChanged(uuid, SharedCursor::Connected);
    }
}

void DeviceConnectManager::handleDeviceDisconnected(TcpSocket *socket)
{
    if (!socket)
        return;

    QUuid uuid = socket->getUuid();

    qDebug() << Q_FUNC_INFO << uuid;

    auto it = devices.find(uuid);
    if (it != devices.end()) {
        disconnectSocket(it.value());
        emit deviceConnectionChanged(uuid, SharedCursor::Disconnected);
    }
    else {
        popTempSocket(socket);
    }
}

void DeviceConnectManager::onSocketConnected(qintptr socketDescriptor)
{
    QSharedPointer<TcpSocket> socket = createSocket();
    socket->setSocketDescriptor(socketDescriptor);
    socket->setType(TcpSocket::Type::ServerOwned);
    pushTempSocket(socket);
}

void DeviceConnectManager::onMessageReceived(const QUuid &uuid, const QJsonObject &json)
{
    const QString &type = json.value(SharedCursor::KEY_TYPE).toString();

    if (type == SharedCursor::KEY_REMOTE_CONTROL) {
        emit remoteControl(QUuid::fromString(json.value(SharedCursor::KEY_MASTER).toString()),
                           QUuid::fromString(json.value(SharedCursor::KEY_SLAVE).toString()));
    }
    else if (type == SharedCursor::KEY_INIT_CURSOR_POS) {
        const QJsonValue &value = json.value(SharedCursor::KEY_VALUE);
        emit cursorInitPosition(SharedCursor::jsonValueToPoint(value));
    }
    else if (type == SharedCursor::KEY_CURSOR_POS) {
        const QJsonValue &value = json.value(SharedCursor::KEY_VALUE);
        emit cursorPosition(SharedCursor::jsonValueToPoint(value));
    }
    else if (type == SharedCursor::KEY_CURSOR_DELTA) {
        const QJsonValue &value = json.value(SharedCursor::KEY_VALUE);
        emit cursorDelta(SharedCursor::jsonValueToPoint(value));
    }
    else if (type == SharedCursor::KEY_INPUT) {
        const QJsonValue &inputType = json.value(SharedCursor::KEY_INPUT);
        int value = json.value(SharedCursor::KEY_VALUE).toInt();

        if (inputType == SharedCursor::KEY_KEYBOARD) {
            emit keyboardEvent(value, json.value(SharedCursor::KEY_PRESSED).toBool());
        }
        else if (inputType == SharedCursor::KEY_MOUSE) {
            emit mouseEvent(value, json.value(SharedCursor::KEY_PRESSED).toBool());
        }
        else if (inputType == SharedCursor::KEY_WHEEL) {
            emit wheelEvent(value);
        }
    }
    else if (type == SharedCursor::KEY_CLIPBOARD) {
        emit clipboard(uuid, json);
    }
}

QJsonObject DeviceConnectManager::devicePtrToJsonObject(QSharedPointer<SharedCursor::Device> device)
{
    QJsonObject result;
    result.insert(SharedCursor::KEY_UUID, device->uuid.toString());
    result.insert(SharedCursor::KEY_NAME, device->name);
    result.insert(SharedCursor::KEY_HOST, QHostAddress(device->host.toIPv4Address()).toString());
    result.insert(SharedCursor::KEY_SELF, device->self);
    result.insert(SharedCursor::KEY_SCREENS, SharedCursor::screenListToJsonValue(device->screens));
    return result;
}

QSharedPointer<SharedCursor::Device> DeviceConnectManager::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<SharedCursor::Device> device = QSharedPointer<SharedCursor::Device>(new SharedCursor::Device);
    device->uuid = QUuid::fromString(obj.value(SharedCursor::KEY_UUID).toString());
    device->name = obj.value(SharedCursor::KEY_NAME).toString();
    device->host = QHostAddress(obj.value(SharedCursor::KEY_HOST).toString());
    device->self = device->uuid == _uuid;
    device->screens = SharedCursor::jsonValueToScreensList(obj.value(SharedCursor::KEY_SCREENS));
    return device;
}

QSharedPointer<TcpSocket> DeviceConnectManager::createSocket()
{
    QSharedPointer<TcpSocket> socket = QSharedPointer<TcpSocket>(new TcpSocket);
    socket->setKeyword(_keyword);

    connect(socket.get(), &TcpSocket::deviceConnected, this, &DeviceConnectManager::handleDeviceConnected, Qt::QueuedConnection);
    connect(socket.get(), &TcpSocket::deviceDisconnected, this, &DeviceConnectManager::handleDeviceDisconnected, Qt::QueuedConnection);
    connect(socket.get(), &TcpSocket::message, this, &DeviceConnectManager::onMessageReceived, Qt::QueuedConnection);

    return socket;
}

void DeviceConnectManager::disconnectSocket(QSharedPointer<TcpSocket> socket)
{
    disconnect(socket.get(), &TcpSocket::deviceConnected, this, &DeviceConnectManager::handleDeviceConnected);
    disconnect(socket.get(), &TcpSocket::deviceDisconnected, this, &DeviceConnectManager::handleDeviceDisconnected);
    disconnect(socket.get(), &TcpSocket::message, this, &DeviceConnectManager::onMessageReceived);
}

void DeviceConnectManager::pushTempSocket(QSharedPointer<TcpSocket> socket)
{
    bool exist = false;
    for (auto &socketPtr: std::as_const(tempSockets)) {
        if (socketPtr.get() == socket) {
            exist = true;
            break;
        }
    }

    if (!exist) {
        tempSockets.append(socket);
    }
}

QSharedPointer<TcpSocket> DeviceConnectManager::popTempSocket(TcpSocket *socket)
{
    QSharedPointer<TcpSocket> result;
    for (auto &socketPtr: std::as_const(tempSockets)) {
        if (socketPtr.get() == socket) {
            result = socketPtr;
            tempSockets.removeOne(socketPtr);
            break;
        }
    }
    return result;
}
