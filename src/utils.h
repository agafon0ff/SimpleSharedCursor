#pragma once

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QObject>
#include <QRect>
#include <QFile>
#include <QUuid>

#include "global.h"
#include "settingsfacade.h"

namespace Utils {

inline bool convertArrayToJson(const QByteArray &data, QJsonObject &result)
{
    QJsonParseError error;
    result = QJsonDocument::fromJson(data, &error).object();
    return error.error == QJsonParseError::NoError;
}

inline void convertJsonToArray(const QJsonObject &jsonObject, QByteArray &result)
{
    QJsonDocument document(jsonObject);
    result = document.toJson(QJsonDocument::Compact);
}

inline QJsonValue pointToJsonValue(const QPoint &point)
{
    QJsonObject result;
    result.insert("x", point.x());
    result.insert("y", point.y());
    return result;
}

inline QPoint jsonValueToPoint(const QJsonValue &jValue)
{
    const QJsonObject &obj = jValue.toObject();
    return QPoint(obj.value("x").toInt(), obj.value("y").toInt());
}

inline QJsonValue rectListToJsonValue(const QVector<QRect> &rectList)
{
    QJsonArray result;
    for (const QRect &rect: rectList) {
        QJsonObject rectObj;
        rectObj.insert("x", rect.x());
        rectObj.insert("y", rect.y());
        rectObj.insert("width", rect.width());
        rectObj.insert("height", rect.height());
        result.append(rectObj);
    }
    return result;
}

inline QVector<QRect> jsonValueToRectList(const QJsonValue &jValue)
{
    QVector<QRect> result;
    QJsonArray arr = jValue.toArray();
    for (const QJsonValue &value: arr) {
        const QJsonObject &obj = value.toObject();
        QRect rect(obj.value("x").toInt(),
                   obj.value("y").toInt(),
                   obj.value("width").toInt(),
                   obj.value("height").toInt());
        result.append(rect);
    }

    return result;
}

inline void fillDeviceJsonMessage(QJsonObject &jsonOut, const char* type)
{
    jsonOut.insert(KEY_TYPE, type);
    jsonOut.insert(KEY_NAME, Settings.name());
    jsonOut.insert(KEY_UUID, Settings.uuid().toString());
    jsonOut.insert(KEY_SCREENS, rectListToJsonValue(Settings.screens()));
}

};
