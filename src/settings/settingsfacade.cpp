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

QSharedPointer<ShareCursor::Device> SettingsFacade::device(const QUuid &uuid) const
{
    return _devices.value(uuid);
}

QMap<QUuid, QSharedPointer<ShareCursor::Device> > SettingsFacade::devices() const
{
    return _devices;
}

QMap<QUuid, QVector<ShareCursor::Transit> > SettingsFacade::transits() const
{
    QMap<QUuid, QVector<ShareCursor::Transit>> result;
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
    saveDevices();
    saveFacadeProperties();
    loader.save(ShareCursor::CONFIG_PATH);
}

void SettingsFacade::loadDevices()
{
    qDebug() << Q_FUNC_INFO;

    const QJsonObject &jsonDevices = loader.value(ShareCursor::KEY_DEVICES).toObject();
    for (auto it=jsonDevices.constBegin(); it != jsonDevices.constEnd(); ++it) {
        QSharedPointer<ShareCursor::Device> device = jsonObjectToDevicePtr(it.value().toObject());
        _devices.insert(device->uuid, device);
        emit deviceFound(device->uuid, device->host);
    }
}

void SettingsFacade::loadFacadeProperties()
{
    _uuid = QUuid::fromString(loader.value(ShareCursor::KEY_UUID).toString());
    _name = loader.value(ShareCursor::KEY_NAME, QHostInfo::localHostName()).toString();
    _keyword = loader.value(ShareCursor::KEY_KEYWORD, QHostInfo::localHostName()).toString();
    _portTcp = loader.value(ShareCursor::KEY_PORT_TCP, ShareCursor::DEFAULT_TCP_PORT).toInt();
    _portUdp = loader.value(ShareCursor::KEY_PORT_UDP, ShareCursor::DEFAULT_UDP_PORT).toInt();
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
    const QUuid &uuid = QUuid::fromString(obj.value(ShareCursor::KEY_UUID).toString());
    if (uuid.isNull())
        return;

    if (_devices.contains(uuid)) {
        fillDeviceProperties(_devices[uuid], obj);
    }
    else {
        QSharedPointer<ShareCursor::Device> device = jsonObjectToDevicePtr(obj);
        _devices.insert(uuid, device);
        emit deviceFound(uuid, device->host);
    }
}

void SettingsFacade::removeDevice(const QUuid &uuid)
{
    if (_devices.contains(uuid)) {
        _devices.remove(uuid);
    }
}

void SettingsFacade::setDevicePosition(const QUuid &uuid, const QPoint &pos)
{
    if (!_devices.contains(uuid))
        return;

    _devices.value(uuid)->position = pos;
}

void SettingsFacade::setTransitsToDevice(const QUuid &uuid, const QVector<ShareCursor::Transit> &transits)
{
    if (!_devices.contains(uuid))
        return;

    _devices.value(uuid)->transits = transits;
}

void SettingsFacade::setDeviceConnectionState(const QUuid &uuid, ShareCursor::ConnectionState state)
{
    if (!_devices.contains(uuid))
        return;

    _devices.value(uuid)->state = state;
}

void SettingsFacade::setValue(const char *key, const QJsonValue &value)
{
    loader.setValue(key, value);
}

QJsonValue SettingsFacade::value(const char *key, const QJsonValue &defaultValue)
{
    return loader.value(key, defaultValue);
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

    loader.setValue(ShareCursor::KEY_DEVICES, jsonDevices);
}

void SettingsFacade::saveSelfDevice()
{
    QString strUuid = loader.value(ShareCursor::KEY_UUID).toString();

    if (strUuid.isEmpty()) {
        strUuid = QUuid::createUuid().toString();
        loader.setValue(ShareCursor::KEY_UUID, strUuid);
    }

    QJsonObject devices = loader.value(ShareCursor::KEY_DEVICES).toObject();
    if (devices.contains(strUuid))
        return;

    QJsonObject device;
    device.insert(ShareCursor::KEY_UUID, QUuid::createUuid().toString());
    device.insert(ShareCursor::KEY_NAME, QHostInfo::localHostName());
    device.insert(ShareCursor::KEY_HOST, "127.0.0.1");
    device.insert(ShareCursor::KEY_SELF, true);
    device.insert(ShareCursor::KEY_UUID, strUuid);
    device.insert(ShareCursor::KEY_SCREENS, Utils::rectListToJsonValue(screens()));
    device.insert(ShareCursor::KEY_POSITION, Utils::pointToJsonValue(QPoint(0, 0)));

    devices.insert(strUuid, device);
    loader.setValue(ShareCursor::KEY_DEVICES, devices);
}

void SettingsFacade::saveFacadeProperties()
{
    loader.setValue(ShareCursor::KEY_UUID, _uuid.toString());
    loader.setValue(ShareCursor::KEY_NAME, _name);
    loader.setValue(ShareCursor::KEY_KEYWORD, _keyword);
    loader.setValue(ShareCursor::KEY_PORT_TCP, _portTcp);
    loader.setValue(ShareCursor::KEY_PORT_UDP, _portUdp);
}

QJsonObject SettingsFacade::devicePtrToJsonObject(QSharedPointer<ShareCursor::Device> device)
{
    QJsonObject result;
    result.insert(ShareCursor::KEY_UUID, device->uuid.toString());
    result.insert(ShareCursor::KEY_NAME, device->name);
    result.insert(ShareCursor::KEY_HOST, QHostAddress(device->host.toIPv4Address()).toString());
    result.insert(ShareCursor::KEY_SELF, device->self);
    result.insert(ShareCursor::KEY_SCREENS, Utils::rectListToJsonValue(device->screens));
    result.insert(ShareCursor::KEY_POSITION, Utils::pointToJsonValue(device->position));
    result.insert(ShareCursor::KEY_TRANSITS, Utils::transitListToJsonValue(device->transits));
    return result;
}

QSharedPointer<ShareCursor::Device> SettingsFacade::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<ShareCursor::Device> device = QSharedPointer<ShareCursor::Device>(new ShareCursor::Device);
    fillDeviceProperties(device, obj);
    return device;
}

void SettingsFacade::fillDeviceProperties(QSharedPointer<ShareCursor::Device> device, const QJsonObject &obj)
{
    device->uuid = QUuid::fromString(obj.value(ShareCursor::KEY_UUID).toString());
    device->self = device->uuid == _uuid;
    device->name = obj.value(ShareCursor::KEY_NAME).toString(device->name);

    if (obj.contains(ShareCursor::KEY_HOST)) {
        device->host = QHostAddress(obj.value(ShareCursor::KEY_HOST).toString());
    }

    if (obj.contains(ShareCursor::KEY_SCREENS)) {
        device->screens = Utils::jsonValueToRectList(obj.value(ShareCursor::KEY_SCREENS));
    }

    if (obj.contains(ShareCursor::KEY_POSITION)) {
        device->position = Utils::jsonValueToPoint(obj.value(ShareCursor::KEY_POSITION));
    }

    if (obj.contains(ShareCursor::KEY_TRANSITS)) {
        device->transits = Utils::jsonValueToTransitList(obj.value(ShareCursor::KEY_TRANSITS));
    }
}

SettingsFacade::SettingsFacade()
{
    loader.load(ShareCursor::CONFIG_PATH, true);
    saveSelfDevice();
}
