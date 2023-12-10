#pragma once

#include <QJsonObject>
#include <QObject>
#include <QMap>
#include "utils.h"

class ClipboardHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardHandler(QObject *parent = nullptr);

public slots:
    void setCurrentUuid(const QUuid &uuid);
    void setRemoteControlState(const QUuid &master, const QUuid &slave);
    void setClipboard(const QUuid &uuid, const QJsonObject &json);

private slots:
    void onClipboardChanged();

signals:
    void message(const QUuid &uuid, const QJsonObject &json);

private:
    QUuid uuidOwn, uuidMaster, uuidSlave;
    QMap<QUuid, bool> clipboardHolders;
    QJsonObject jsonMessage;
    bool applyingClipboard = false;

    void sendClipboard(const QUuid &uuid);
};
