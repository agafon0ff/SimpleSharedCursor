#pragma once

#include <QObject>
#include <QJsonObject>
#include <QMutex>

#include "jsonloader.h"
#include "global.h"

class SettingsFacade : public QObject
{
    Q_OBJECT
public:
    static SettingsFacade &instance();

    QUuid uuid() const;
    QString keyword() const;

public slots:
    void load();
    void save();

    void setKeyword(const QString &keyword);

    void setValue(const char *key, const QJsonValue &value);
    QJsonValue value(const char* key, const QJsonValue &defaultValue = QJsonValue());

signals:
    void settingsChanged(const char *key);

private:
    QMutex mutex;
    JsonLoader loader;

    QUuid _uuid;
    QString _keyword;
    quint16 _tcpPort = DEFAULT_TCP_PORT;
    quint16 _udpPort = DEFAULT_UDP_PORT;
    QMap<QUuid, QSharedPointer<Device>> _devices;

    void loadDevices();
    void saveDevices();

    void saveSelfDevice();
    void loadFacadeProperties();

    QJsonObject devicePtrToJsonObject(QSharedPointer<Device> device);
    QSharedPointer<Device> jsonObjectToDevicePtr(const QJsonObject &obj);

    SettingsFacade();
    ~SettingsFacade();

    SettingsFacade(const SettingsFacade &in) = delete;
    SettingsFacade(SettingsFacade &&in) = delete;
    SettingsFacade& operator=(const SettingsFacade &in) = delete;
    void* operator new[](std::size_t) = delete;
    void operator delete[](void*) = delete;
};

#define Settings SettingsFacade::instance()
