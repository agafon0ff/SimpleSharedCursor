#include <QJsonArray>

#include "deviceconnectmanager.h"
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

    if (uuid.isNull() || uuid == _uuid)
        return;

    auto it = devices.find(uuid);
    if (it != devices.end()) {
        QSharedPointer<TcpSocket> socket = devices.value(uuid);
        if(!socket.isNull() && socket->isConnected()) {
            return;
        } else {
            it.value().clear();
        }
    }

    QSharedPointer<TcpSocket> socket = createTempSocket();
    socket->setType(TcpSocket::Type::Independent);
    socket->setUuid(uuid);
    socket->setHost(host);
    socket->setPort(_port);
    socket->start();

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
    jsonRemoteControl[SharedCursor::KEY_TYPE] = SharedCursor::KEY_REMOTE_CONTROL;
    jsonRemoteControl[SharedCursor::KEY_MASTER] = master.toString();
    jsonRemoteControl[SharedCursor::KEY_SLAVE] = slave.toString();

    qDebug() << Q_FUNC_INFO << jsonRemoteControl;

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
    qDebug() << Q_FUNC_INFO;

    QUuid uuid = QUuid::fromString(json.value(SharedCursor::KEY_UUID).toString());
    QSharedPointer<TcpSocket> socketPtr = popTempSocket(socket);

    if (socketPtr.isNull())
        return;

    auto it = devices.find(uuid);
    if (it != devices.end()) {
        it.value().swap(socketPtr);
    } else {
        devices.insert(uuid, socketPtr);
    }

    emit deviceConnectionChanged(uuid, SharedCursor::Connected);
}

void DeviceConnectManager::handleDeviceDisconnected(TcpSocket *socket)
{
    if (!socket)
        return;

    QUuid uuid = socket->getUuid();

    qDebug() << Q_FUNC_INFO << uuid;

    auto it = devices.find(uuid);
    if (it != devices.end()) {
        QSharedPointer<TcpSocket> socket = it.value();
        disconnect(socket.get(), &TcpSocket::deviceConnected, this, &DeviceConnectManager::handleDeviceConnected);
        disconnect(socket.get(), &TcpSocket::deviceDisconnected, this, &DeviceConnectManager::handleDeviceDisconnected);
        disconnect(socket.get(), &TcpSocket::message, this, &DeviceConnectManager::onMessageReceived);

        emit deviceConnectionChanged(uuid, SharedCursor::Disconnected);
        it.value().clear();
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

void DeviceConnectManager::onMessageReceived(const QUuid &uuid, const QJsonObject &json)
{
    Q_UNUSED(uuid);
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
    result.insert(SharedCursor::KEY_SCREENS, SharedCursor::rectListToJsonValue(device->screens));
    return result;
}

QSharedPointer<SharedCursor::Device> DeviceConnectManager::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<SharedCursor::Device> device = QSharedPointer<SharedCursor::Device>(new SharedCursor::Device);
    device->uuid = QUuid::fromString(obj.value(SharedCursor::KEY_UUID).toString());
    device->name = obj.value(SharedCursor::KEY_NAME).toString();
    device->host = QHostAddress(obj.value(SharedCursor::KEY_HOST).toString());
    device->self = device->uuid == _uuid;
    device->screens = SharedCursor::jsonValueToRectList(obj.value(SharedCursor::KEY_SCREENS));
    return device;
}

QSharedPointer<TcpSocket> DeviceConnectManager::createTempSocket()
{
    QSharedPointer<TcpSocket> socket = QSharedPointer<TcpSocket>(new TcpSocket);
    socket->setKeyword(_keyword);
    tempSockets.append(socket);

    connect(socket.get(), &TcpSocket::deviceConnected, this, &DeviceConnectManager::handleDeviceConnected, Qt::QueuedConnection);
    connect(socket.get(), &TcpSocket::deviceDisconnected, this, &DeviceConnectManager::handleDeviceDisconnected, Qt::QueuedConnection);
    connect(socket.get(), &TcpSocket::message, this, &DeviceConnectManager::onMessageReceived, Qt::QueuedConnection);

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
