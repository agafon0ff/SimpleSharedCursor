#pragma once

#include <QJsonObject>

class JsonLoader
{
public:
    void load(const char *path, bool backup = false);
    void save(const char *path);

    void setValue(const char *key, const QJsonValue &value);
    QJsonValue value(const char* key, const QJsonValue &defaultValue = QJsonValue());

private:
    QJsonObject values;
};
