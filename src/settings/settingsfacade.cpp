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

QSharedPointer<Device> SettingsFacade::device(const QUuid &uuid) const
{
    return _devices.value(uuid);
}

QMap<QUuid, QSharedPointer<Device> > SettingsFacade::devices() const
{
    return _devices;
}

void SettingsFacade::load()
{
    loader.load(CONFIG_PATH, true);
    saveSelfDevice();
    loadFacadeProperties();
    loadDevices();
}

void SettingsFacade::save()
{
    saveDevices();
    saveFacadeProperties();
    loader.save(CONFIG_PATH);
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

void SettingsFacade::setDevice(const QJsonObject &obj)
{
    const QUuid &uuid = QUuid::fromString(obj.value(KEY_UUID).toString());
    if (uuid.isNull())
        return;

    if (_devices.contains(uuid)) {
        fillDeviceProperties(_devices[uuid], obj);
    }
    else {
        _devices.insert(uuid, jsonObjectToDevicePtr(obj));
    }
}

void SettingsFacade::setDevicePosition(const QUuid &uuid, const QPoint &pos)
{
    if (!_devices.contains(uuid))
        return;

    _devices.value(uuid)->position = pos;
}

void SettingsFacade::setTransitsToDevice(const QUuid &uuid, const QVector<Transit> &transits)
{
    if (!_devices.contains(uuid))
        return;

    _devices.value(uuid)->transits = transits;
}

void SettingsFacade::setValue(const char *key, const QJsonValue &value)
{
    loader.setValue(key, value);
}

QJsonValue SettingsFacade::value(const char *key, const QJsonValue &defaultValue)
{
    return loader.value(key, defaultValue);
}

void SettingsFacade::loadDevices()
{
    qDebug() << Q_FUNC_INFO;

    const QJsonObject &jsonDevices = loader.value(KEY_DEVICES).toObject();
    for (auto it=jsonDevices.constBegin(); it != jsonDevices.constEnd(); ++it) {
        QSharedPointer<Device> device = jsonObjectToDevicePtr(it.value().toObject());
        _devices.insert(device->uuid, device);
    }
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

    loader.setValue(KEY_DEVICES, jsonDevices);
}

void SettingsFacade::saveSelfDevice()
{
    QString strUuid = loader.value(KEY_UUID).toString();

    if (strUuid.isEmpty()) {
        strUuid = QUuid::createUuid().toString();
        loader.setValue(KEY_UUID, strUuid);
    }

    QJsonObject devices = loader.value(KEY_DEVICES).toObject();
    if (devices.contains(strUuid))
        return;

    QJsonObject device;
    device.insert(KEY_UUID, QUuid::createUuid().toString());
    device.insert(KEY_NAME, QHostInfo::localHostName());
    device.insert(KEY_HOST, "127.0.0.1");
    device.insert(KEY_SELF, true);
    device.insert(KEY_UUID, strUuid);
    device.insert(KEY_SCREENS, Utils::rectListToJsonValue(screens()));
    device.insert(KEY_POSITION, Utils::pointToJsonValue(QPoint(0, 0)));

    devices.insert(strUuid, device);
    loader.setValue(KEY_DEVICES, devices);
}

void SettingsFacade::loadFacadeProperties()
{
    _uuid = QUuid::fromString(loader.value(KEY_UUID).toString());
    _name = loader.value(KEY_NAME, QHostInfo::localHostName()).toString();
    _keyword = loader.value(KEY_KEYWORD, QHostInfo::localHostName()).toString();
    _portTcp = loader.value(KEY_PORT_TCP, DEFAULT_TCP_PORT).toInt();
    _portUdp = loader.value(KEY_PORT_UDP, DEFAULT_UDP_PORT).toInt();
}

void SettingsFacade::saveFacadeProperties()
{
    loader.setValue(KEY_UUID, _uuid.toString());
    loader.setValue(KEY_NAME, _name);
    loader.setValue(KEY_KEYWORD, _keyword);
    loader.setValue(KEY_PORT_TCP, _portTcp);
    loader.setValue(KEY_PORT_UDP, _portUdp);
}

QJsonObject SettingsFacade::devicePtrToJsonObject(QSharedPointer<Device> device)
{
    QJsonObject result;
    result.insert(KEY_UUID, device->uuid.toString());
    result.insert(KEY_NAME, device->name);
    result.insert(KEY_HOST, QHostAddress(device->host.toIPv4Address()).toString());
    result.insert(KEY_SELF, device->self);
    result.insert(KEY_SCREENS, Utils::rectListToJsonValue(device->screens));
    result.insert(KEY_POSITION, Utils::pointToJsonValue(device->position));
    result.insert(KEY_TRANSITS, Utils::transitListToJsonValue(device->transits));
    return result;
}

QSharedPointer<Device> SettingsFacade::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<Device> device = QSharedPointer<Device>(new Device);
    fillDeviceProperties(device, obj);
    return device;
}

void SettingsFacade::fillDeviceProperties(QSharedPointer<Device> device, const QJsonObject &obj)
{
    device->uuid = QUuid::fromString(obj.value(KEY_UUID).toString());
    device->self = device->uuid == _uuid;
    device->name = obj.value(KEY_NAME).toString(device->name);

    if (obj.contains(KEY_HOST)) {
        device->host = QHostAddress(obj.value(KEY_HOST).toString());
    }

    if (obj.contains(KEY_SCREENS)) {
        device->screens = Utils::jsonValueToRectList(obj.value(KEY_SCREENS));
    }

    if (obj.contains(KEY_POSITION)) {
        device->position = Utils::jsonValueToPoint(obj.value(KEY_POSITION));
    }

    if (obj.contains(KEY_TRANSITS)) {
        device->transits = Utils::jsonValueToTransitList(obj.value(KEY_TRANSITS));
    }
}

SettingsFacade::SettingsFacade()
{
    load();
}
