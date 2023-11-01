#include <QJsonDocument>
#include <QMutexLocker>
#include <QFileInfo>
#include <QFile>
#include <QDir>

#include "settingsloader.h"

const char* CONFIG_PATH = "config.json";
const char* CONFIG_BACKUP_PATH = "config.json~";
const char* PROGRAM_NAME = "ShareCursor";

SettingsLoader &SettingsLoader::instance()
{
    static SettingsLoader * const ptr = new SettingsLoader();
    return *ptr;
}

void SettingsLoader::load()
{
    QByteArray jsonData;
    QFile configFile(CONFIG_PATH);
    QFile backupFile(CONFIG_BACKUP_PATH);

    if(configFile.open(QIODevice::ReadOnly))
    {
        jsonData.resize(configFile.size());
        configFile.read(jsonData.data(), jsonData.size());
        configFile.close();
    }

    QJsonParseError jsonError;
    values = QJsonDocument::fromJson(jsonData, &jsonError).object();

    if(jsonError.error != QJsonParseError::NoError)
    {
        if(backupFile.open(QIODevice::ReadOnly))
        {
            jsonData.resize(backupFile.size());
            backupFile.read(jsonData.data(), jsonData.size());
            backupFile.close();
        }

        values = QJsonDocument::fromJson(jsonData, &jsonError).object();
    }

    if(jsonError.error != QJsonParseError::NoError)
        values = QJsonObject();

    if(backupFile.open(QIODevice::WriteOnly))
    {
        backupFile.write(jsonData);
        backupFile.close();
    }
}

void SettingsLoader::save()
{
    QFileInfo fInfo(CONFIG_PATH);
    if(!fInfo.dir().exists())
    {
        QDir dir;
        dir.mkpath(fInfo.dir().path());
    }

    QFile file(CONFIG_PATH);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(QJsonDocument(values).toJson());
        file.close();
    }
}

void SettingsLoader::setValue(const char *key, const QJsonValue &value)
{
    QMutexLocker locker(&mutex);

    if (values.contains(key)) {
        values[key] = value;
    }
    else {
        values.insert(key, value);
    }

    save();
}

QJsonValue SettingsLoader::value(const char *key, const QJsonValue &defaultValue)
{
    if(values.contains(key)) {
        return values.value(key);
    }
    else {
        if(!defaultValue.isNull())
            setValue(key, defaultValue);
    }

    return defaultValue;
}

SettingsLoader::SettingsLoader()
{
    load();
}

SettingsLoader::~SettingsLoader()
{
    save();
}
