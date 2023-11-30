#pragma once

#include <QObject>
#include <QJsonObject>
#include <QSharedPointer>
#include "utils.h"

class CursorHandler : public QObject
{
    Q_OBJECT
public:
    explicit CursorHandler(QObject *parent = nullptr);
    ~CursorHandler();

public slots:
    void start();
    void stop();

    void setHoldCursorPosition(const QPoint &pos);
    void setCurrentUuid(const QUuid &uuid);
    void setTransits(const QMap<QUuid, QVector<ShareCursor::Transit>> &transits);
    void setConnectionState(const QUuid &uuid, ShareCursor::ConnectionState state);
    void setRemoteCursorPos(const QUuid &uuid, const QPoint &pos);
    void setControlledByUuid(const QUuid &uuid);

signals:
    void started();
    void finished();
    void controlRemoteDevice(bool state);
    void message(const QUuid &uuid, const QJsonObject &json);

private:
    int timerId = 0;
    ShareCursor::ControlState controlState = ShareCursor::SelfControl;
    ShareCursor::ConnectionState currentTransitState = ShareCursor::Unknown;
    QUuid transitUuid;
    QUuid currentDeviceUuid;
    QUuid controlledByUuid;
    QJsonObject jsonDelta, jsonPosition, jsonRemoteControl;
    QPoint lastCursorPosition = {0, 0};
    QPoint holdCursorPosition = {0, 0};
    QVector<ShareCursor::Transit> currentTransits;
    QMap<QUuid, QVector<ShareCursor::Transit>> transitsMap;
    QMap<QUuid, ShareCursor::ConnectionState> connnetionStates;

    void timerEvent(QTimerEvent *e);
    void checkCursor(const QPoint &pos);
    void sendCursorDelta(const QPoint &pos);
    void sendCursorPosition(const QPoint &pos);
    void setCursorPosition(const QPoint &pos);
    void sendRemoteControlMessage(bool state, const QPoint &pos);

    QPoint calculateRemotePos(const ShareCursor::Transit &transit, const QPoint &pos);
};
