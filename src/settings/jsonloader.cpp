#include <QJsonDocument>
#include <QFileInfo>
#include <QObject>
#include <QFile>
#include <QDir>

#include "jsonloader.h"

void JsonLoader::load(const char* path, bool backup)
{
    QByteArray jsonData;
    QFile configFile(path);

    QFile backupFile(configFile.fileName() + "~");

    if(configFile.open(QIODevice::ReadOnly))
    {
        jsonData.resize(configFile.size());
        configFile.read(jsonData.data(), jsonData.size());
        configFile.close();
    }

    QJsonParseError jsonError;
    values = QJsonDocument::fromJson(jsonData, &jsonError).object();

    if (!backup)
        return;

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

void JsonLoader::save(const char* path)
{
    QFileInfo fInfo(path);
    if(!fInfo.dir().exists())
    {
        QDir dir;
        dir.mkpath(fInfo.dir().path());
    }

    QFile file(path);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(QJsonDocument(values).toJson());
        file.close();
    }
}

void JsonLoader::setValue(const char *key, const QJsonValue &value)
{
    if (values.contains(key)) {
        values[key] = value;
    }
    else {
        values.insert(key, value);
    }
}

QJsonValue JsonLoader::value(const char *key, const QJsonValue &defaultValue)
{
    if(values.contains(key)) {
        return values.value(key);
    }

    if(!defaultValue.isNull()) {
        setValue(key, defaultValue);
    }

    return defaultValue;
}
