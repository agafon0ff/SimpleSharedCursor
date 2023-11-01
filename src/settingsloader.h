#pragma once

#include <QObject>
#include <QJsonObject>
#include <QMutex>

class SettingsLoader : public QObject
{
    Q_OBJECT
public:
    static SettingsLoader &instance();

signals:


public slots:
    void load();
    void save();

    void setValue(const char *key, const QJsonValue &value);
    QJsonValue value(const char* key, const QJsonValue &defaultValue = QJsonValue());

private:
    QMutex mutex;
    QJsonObject values;

    SettingsLoader();
    ~SettingsLoader();

    SettingsLoader(const SettingsLoader &in) = delete;
    SettingsLoader(SettingsLoader &&in) = delete;
    SettingsLoader& operator=(const SettingsLoader &in) = delete;
    void* operator new[](std::size_t) = delete;
    void operator delete[](void*) = delete;
};

#define Settings SettingsLoader::instance()
