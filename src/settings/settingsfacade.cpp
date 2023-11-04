#include <QGuiApplication>
#include <QMutexLocker>
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

QString SettingsFacade::keyword() const
{
    return _keyword;
}

void SettingsFacade::load()
{
    loader.load(CONFIG_PATH);
    saveSelfDevice();
    loadFacadeProperties();
}

void SettingsFacade::save()
{
    loader.save(CONFIG_PATH);
}

void SettingsFacade::setKeyword(const QString &keyword)
{
    _keyword = keyword;
    setValue(KEY_KEYWORD, _keyword);
    emit settingsChanged(KEY_KEYWORD);
}

void SettingsFacade::setValue(const char *key, const QJsonValue &value)
{
    QMutexLocker locker(&mutex);
    loader.setValue(key, value);
    emit settingsChanged(key);
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

    emit settingsChanged(KEY_DEVICES);
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
    emit settingsChanged(KEY_DEVICES);
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

    QVector<QRect> resolutions;
    QList <QScreen*> screens = QGuiApplication::screens();
    for (QScreen *screen: qAsConst(screens))
        resolutions.append(screen->geometry());

    device.insert(KEY_SCREENS, Utils::rectListToJsonValue(resolutions));
    device.insert(KEY_POSITION, Utils::pointToJsonValue(QPoint(0, 0)));

    devices.insert(strUuid, device);
    loader.setValue(KEY_DEVICES, devices);
}

void SettingsFacade::loadFacadeProperties()
{
    _uuid = loader.value(KEY_UUID).toString();
    _keyword = loader.value(KEY_KEYWORD, QHostInfo::localHostName()).toString();
}

QJsonObject SettingsFacade::devicePtrToJsonObject(QSharedPointer<Device> device)
{
    QJsonObject result;
    result.insert(KEY_UUID, device->uuid.toString());
    result.insert(KEY_NAME, device->name);
    result.insert(KEY_HOST, QHostAddress(device->host.toIPv4Address()).toString());
    result.insert(KEY_SELF, device->self);
    result.insert(KEY_SCREENS, Utils::rectListToJsonValue(device->screens));
    return result;
}

QSharedPointer<Device> SettingsFacade::jsonObjectToDevicePtr(const QJsonObject &obj)
{
    QSharedPointer<Device> device = QSharedPointer<Device>(new Device);
    device->uuid = QUuid::fromString(obj.value(KEY_UUID).toString());
    device->name = obj.value(KEY_NAME).toString();
    device->host = QHostAddress(obj.value(KEY_HOST).toString());
    device->self = device->uuid == _uuid;
    device->screens = Utils::jsonValueToRectList(obj.value(KEY_SCREENS));
    return device;
}

SettingsFacade::SettingsFacade()
{
    load();
}

SettingsFacade::~SettingsFacade()
{
    save();
}
