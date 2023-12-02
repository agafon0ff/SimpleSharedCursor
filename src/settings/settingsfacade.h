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
    QRect screenRect();
    QSharedPointer<SharedCursor::Device> device(const QUuid &uuid) const;
    QMap<QUuid, QSharedPointer<SharedCursor::Device>> devices() const;
    QMap<QUuid, QVector<SharedCursor::Transit>> transits() const;

public slots:

    void save();
    void loadDevices();
    void loadFacadeProperties();

    void setName(const QString &name);
    void setKeyword(const QString &keyword);
    void setPortTcp(quint16 port);
    void setPortUdp(quint16 port);
    void setDevicePosition(const QUuid &uuid, const QPoint &pos);
    void setDeviceConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state);

    void clearTransits();
    void setTransitsToDevice(const QUuid &uuid, const QVector<SharedCursor::Transit> &transits);
    void addTransitsToDevice(const QUuid &uuid, const QVector<SharedCursor::Transit> &transits);

    void setDevice(const QJsonObject &obj);
    void removeDevice(const QUuid &uuid);

    void setValue(const char *key, const QJsonValue &value);
    QJsonValue value(const char* key, const QJsonValue &defaultValue = QJsonValue());

signals:
    void deviceFound(const QUuid &uuid, const QHostAddress &host);


private:
    JsonLoader _loader;

    QUuid _uuid;
    QString _name;
    QString _keyword;
    quint16 _portTcp = SharedCursor::DEFAULT_TCP_PORT;
    quint16 _portUdp = SharedCursor::DEFAULT_UDP_PORT;
    QMap<QUuid, QSharedPointer<SharedCursor::Device>> _devices;

    void saveDevices();
    void saveSelfDevice();
    void saveFacadeProperties();

    QJsonObject devicePtrToJsonObject(QSharedPointer<SharedCursor::Device> device);
    QSharedPointer<SharedCursor::Device> jsonObjectToDevicePtr(const QJsonObject &obj);
    void fillDeviceProperties(QSharedPointer<SharedCursor::Device> device, const QJsonObject &obj);

    SettingsFacade();

    SettingsFacade(const SettingsFacade &in) = delete;
    SettingsFacade(SettingsFacade &&in) = delete;
    SettingsFacade& operator=(const SettingsFacade &in) = delete;
    void* operator new[](std::size_t) = delete;
    void operator delete[](void*) = delete;
};

#define Settings SettingsFacade::instance()
