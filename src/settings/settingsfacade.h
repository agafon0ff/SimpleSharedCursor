#pragma once

#include <QObject>
#include <QJsonObject>

#include "jsonloader.h"
#include "global.h"

class SettingsFacade : public QObject
{
    Q_OBJECT
public:
    static SettingsFacade &instance();

    QUuid uuid() const;
    QString name() const;
    QString keyword() const;
    quint16 portTcp() const;
    quint16 portUdp() const;
    QVector<QRect> screens();
    QSharedPointer<Device> device(const QUuid &uuid) const;
    QMap<QUuid, QSharedPointer<Device>> devices() const;

public slots:
    void load();
    void save();

    void setName(const QString &name);
    void setKeyword(const QString &keyword);
    void setPortTcp(quint16 port);
    void setPortUdp(quint16 port);
    void setDevice(const QJsonObject &obj);
    void setDevicePosition(const QUuid &uuid, const QPoint &pos);
    void setTransitsToDevice(const QUuid &uuid, const QVector<Transit> &transits);

    void setValue(const char *key, const QJsonValue &value);
    QJsonValue value(const char* key, const QJsonValue &defaultValue = QJsonValue());

private:
    JsonLoader loader;

    QUuid _uuid;
    QString _name;
    QString _keyword;
    quint16 _portTcp = DEFAULT_TCP_PORT;
    quint16 _portUdp = DEFAULT_UDP_PORT;
    QMap<QUuid, QSharedPointer<Device>> _devices;

    void loadDevices();
    void saveDevices();

    void saveSelfDevice();

    void loadFacadeProperties();
    void saveFacadeProperties();

    QJsonObject devicePtrToJsonObject(QSharedPointer<Device> device);
    QSharedPointer<Device> jsonObjectToDevicePtr(const QJsonObject &obj);
    void fillDeviceProperties(QSharedPointer<Device> device, const QJsonObject &obj);

    SettingsFacade();

    SettingsFacade(const SettingsFacade &in) = delete;
    SettingsFacade(SettingsFacade &&in) = delete;
    SettingsFacade& operator=(const SettingsFacade &in) = delete;
    void* operator new[](std::size_t) = delete;
    void operator delete[](void*) = delete;
};

#define Settings SettingsFacade::instance()
