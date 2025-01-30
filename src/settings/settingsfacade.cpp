#include <QGuiApplication>
#include <QHostInfo>
#include <QScreen>
#include <QDebug>
#include "settingsfacade.h"
#include "utils.h"

SettingsFacade &SettingsFacade::instance()
{
    static SettingsFacade * const ptr = new SettingsFacade();
    return *ptr;
}

QUuid SettingsFacade::uuid() const
{
    return _uuid;
}

QString SettingsFacade::name() const
{
    return _name;
}

QString SettingsFacade::keyword() const
{
    return _keyword;
}

quint16 SettingsFacade::portTcp() const
{
    return _portTcp;
}

quint16 SettingsFacade::portUdp() const
{
    return _portUdp;
}

QVector<QRect> SettingsFacade::screens()
{
    QVector<QRect> result;
    const QList <QScreen*> &screens = QGuiApplication::screens();

    for (QScreen *screen: screens) {
        result.append(screen->geometry());
    }

    return result;
}

QRect SettingsFacade::screenRect()
{
    return QGuiApplication::primaryScreen()->geometry();
}

QSharedPointer<SharedCursor::Device> SettingsFacade::device(const QUuid &uuid) const
{
    return _devices.value(uuid);
}

QMap<QUuid, QSharedPointer<SharedCursor::Device> > SettingsFacade::devices() const
{
    return _devices;
}

QMap<QUuid, QVector<SharedCursor::Transit> > SettingsFacade::transits() const
{
    QMap<QUuid, QVector<SharedCursor::Transit>> result;
    auto i = _devices.constBegin();
    while (i != _devices.constEnd()) {
        auto &device = i.value();
        result.insert(device->uuid, device->transits);
        ++i;
    }
    return result;
}

void SettingsFacade::save()
{
    qDebug() << Q_FUNC_INFO;

    saveDevices();
    saveFacadeProperties();
    _loader.save(SharedCursor::CONFIG_PATH);
}

void SettingsFacade::loadDevices()
{
    qDebug() << Q_FUNC_INFO;

    const QJsonObject &jsonDevices = _loader.value(SharedCursor::KEY_DEVICES).toObject();
    for (auto it=jsonDevices.constBegin(); it != jsonDevices.constEnd(); ++it) {
        QSharedPointer<SharedCursor::Device> device = jsonObjectToDevicePtr(it.value().toObject());
        _devices.insert(device->uuid, device);
        emit deviceFound(device->uuid, device->host);
    }
}

void SettingsFacade::loadFacadeProperties()
{
    _uuid = QUuid::fromString(_loader.value(SharedCursor::KEY_UUID).toString());
    _name = _loader.value(SharedCursor::KEY_NAME, QHostInfo::localHostName()).toString();
    _keyword = _loader.value(SharedCursor::KEY_KEYWORD, QHostInfo::localHostName()).toString();
    _portTcp = _loader.value(SharedCursor::KEY_PORT_TCP, SharedCursor::DEFAULT_TCP_PORT).toInt();
    _portUdp = _loader.value(SharedCursor::KEY_PORT_UDP, SharedCursor::DEFAULT_UDP_PORT).toInt();
}

void SettingsFacade::setName(const QString &name)
{
    _name = name;
}

void SettingsFacade::setKeyword(const QString &keyword)
{
    _keyword = keyword;
}

void SettingsFacade::setPortTcp(quint16 port)
{
    _portTcp = port;
}

void SettingsFacade::setPortUdp(quint16 port)
{
    _portUdp = port;
}

void SettingsFacade::clearDevices()
{
    _devices.clear();
}

void SettingsFacade::setDevice(const QJsonObject &obj)
{
    const QUuid &uuid = QUuid::fromString(obj.value(SharedCursor::KEY_UUID).toString());
    if (uuid.isNull())
        return;

    if (_devices.contains(uuid)) {
        fillDeviceProperties(_devices[uuid], obj);
    }
    else {
        QSharedPointer<SharedCursor::Device> device = jsonObjectToDevicePtr(obj);
        _devices.insert(uuid, device);
        emit deviceFound(uuid, device->host);
    }
}

void SettingsFacade::removeDevice(const QUuid &uuid)
{
    _devices.remove(uuid);
}

void SettingsFacade::setDevicePosition(const QUuid &uuid, const QPoint &pos)
{
    auto it = _devices.find(uuid);
    if (it != _devices.end()) {
        it.value()->position = pos;
    }
}

void SettingsFacade::setDeviceConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state)
{
    auto it = _devices.find(uuid);
    if (it != _devices.end()) {
        it.value()->state = state;
    }
}

void SettingsFacade::clearTransits()
{
    auto i = _devices.constBegin();
    while (i != _devices.constEnd()) {
        i.value()->transits.clear();
        ++i;
    }
}

void SettingsFacade::setTransitsToDevice(const QUuid &uuid, const QVector<SharedCursor::Transit> &transits)
{
    auto it = _devices.find(uuid);
    if (it != _devices.end()) {
        it.value()->transits = transits;
    }
}

void SettingsFacade::addTransitsToDevice(const QUuid &uuid, const QVector<SharedCursor::Transit> &transits)
{
    auto it = _devices.find(uuid);
    if (it != _devices.end()) {
        it.value()->transits.append(transits);
    }
}

void SettingsFacade::setValue(const char *key, const QJsonValue &value)
{
    _loader.setValue(key, value);
}

QJsonValue SettingsFacade::value(const char *key, const QJsonValue &defaultValue)
{
    return _loader.value(key, defaultValue);
}

void SettingsFacade::saveDevices()
{
    QJsonObject jsonDevices;
    auto i = _devices.constBegin();
    while (i != _devices.constEnd()) {
        auto &device = i.value();
        jsonDevices.insert(device->uuid.toString(), devicePtrToJsonObject(i.value()));
        ++i;
    }

    _loader.setValue(SharedCursor::KEY_DEVICES, jsonDevices);
}

void SettingsFacade::saveSelfDevice()
{
    QString strUuid = _loader.value(SharedCursor::KEY_UUID).toString();

    if (strUuid.isEmpty()) {
        strUuid = QUuid::createUuid().toString();
        _loader.setValue(SharedCursor::KEY_UUID, strUuid);
    }

    QJsonObject devices = _loader.value(SharedCursor::KEY_DEVICES).toObject();
    if (devices.contains(strUuid))
        return;

    QJsonObject device;
    device.insert(SharedCursor::KEY_UUID, QUuid::createUuid().toString());
    device.insert(SharedCursor::KEY_NAME, QHostInfo::localHostName());
    device.insert(SharedCursor::KEY_HOST, "127.0.0.1");
    device.insert(SharedCursor::KEY_SELF, true);
    device.insert(SharedCursor::KEY_UUID, strUuid);
    device.insert(SharedCursor::KEY_SCREENS, SharedCursor::rectListToJsonValue(screens()));
    device.insert(SharedCursor::KEY_POSITION, SharedCursor::pointToJsonValue(QPoint(0, 0)));

    devices.insert(strUuid, device);
    _loader.setValue(SharedCursor::KEY_DEVICES, devices);
}

void SettingsFacade::saveFacadeProperties()
{
    _loader.setValue(SharedCursor::KEY_UUID, _uuid.toString());
    _loader.setValue(SharedCursor::KEY_NAME, _name);
    _loader.setValue(SharedCursor::KEY_KEYWORD, _keyword);
    _loader.setValue(SharedCursor::KEY_PORT_TCP, _portTcp);
    _loader.setValue(SharedCursor::KEY_PORT_UDP, _portUdp);
}

QJsonObject SettingsFacade::devicePtrToJsonObject(QSharedPointer<SharedCursor::Device> device)
{
    QJsonObject result;
    result.insert(SharedCursor::KEY_UUID, device->uuid.toString());
    result.insert(SharedCursor::KEY_NAME, device->name);
    result.insert(SharedCursor::KEY_HOST, QHostAddress(device->host.toIPv4Address()).toString());
    result.insert(SharedCursor::KEY_SELF, device->self);
    result.insert(SharedCursor::KEY_SCREENS, SharedCursor::rectListToJsonValue(device->screens));
    result.insert(SharedCursor::KEY_POSITION, SharedCursor::pointToJsonValue(device->position));
    result.insert(SharedCursor::KEY_TRANSITS, SharedCursor::transitListToJsonValue(device->transits));
    return result;
}

QSharedPointer<SharedCursor::Device> SettingsFacade::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<SharedCursor::Device> device = QSharedPointer<SharedCursor::Device>(new SharedCursor::Device);
    fillDeviceProperties(device, obj);
    return device;
}

void SettingsFacade::fillDeviceProperties(QSharedPointer<SharedCursor::Device> device, const QJsonObject &obj)
{
    device->uuid = QUuid::fromString(obj.value(SharedCursor::KEY_UUID).toString());
    device->self = device->uuid == _uuid;
    device->name = obj.value(SharedCursor::KEY_NAME).toString(device->name);

    if (obj.contains(SharedCursor::KEY_HOST)) {
        device->host = QHostAddress(obj.value(SharedCursor::KEY_HOST).toString());
    }

    if (obj.contains(SharedCursor::KEY_SCREENS)) {
        device->screens = SharedCursor::jsonValueToRectList(obj.value(SharedCursor::KEY_SCREENS));
    }

    if (obj.contains(SharedCursor::KEY_POSITION)) {
        device->position = SharedCursor::jsonValueToPoint(obj.value(SharedCursor::KEY_POSITION));
    }

    if (obj.contains(SharedCursor::KEY_TRANSITS)) {
        device->transits = SharedCursor::jsonValueToTransitList(obj.value(SharedCursor::KEY_TRANSITS));
    }
}

SettingsFacade::SettingsFacade()
{
    _loader.load(SharedCursor::CONFIG_PATH, true);
    saveSelfDevice();
}
