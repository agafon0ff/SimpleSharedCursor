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
    void setTransits(const QMap<QUuid, QVector<SharedCursor::Transit>> &transits);
    void setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state);
    void setRemoteCursorPos(const QUuid &uuid, const QPoint &pos);
    void setControlledByUuid(const QUuid &uuid);

signals:
    void started();
    void finished();
    void controlRemoteDevice(const QUuid &uuid, bool state);
    void message(const QUuid &uuid, const QJsonObject &json);

private:
    int timerId = 0;
    SharedCursor::ControlState controlState = SharedCursor::SelfControl;
    SharedCursor::ConnectionState currentTransitState = SharedCursor::Unknown;
    QUuid transitUuid;
    QUuid currentDeviceUuid;
    QUuid controlledByUuid;
    QJsonObject jsonDelta, jsonPosition, jsonRemoteControl;
    QPoint lastCursorPosition = {0, 0};
    QPoint holdCursorPosition = {0, 0};
    QVector<SharedCursor::Transit> currentTransits;
    QMap<QUuid, QVector<SharedCursor::Transit>> transitsMap;
    QMap<QUuid, SharedCursor::ConnectionState> connnetionStates;

    void timerEvent(QTimerEvent *e);
    void checkCursor(const QPoint &pos);
    void sendCursorDelta(const QPoint &pos);
    void sendCursorPosition(const QPoint &pos);
    void setCursorPosition(const QPoint &pos);
    void sendRemoteControlMessage(bool state, const QPoint &pos);

    QPoint calculateRemotePos(const SharedCursor::Transit &transit, const QPoint &pos);
};
