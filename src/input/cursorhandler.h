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
    void setDevices(const QMap<QUuid, QSharedPointer<SharedCursor::Device>> &_devices);

    void setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state);
    void setRemoteCursorPos(const QPoint &pos);
    void setRemoteControlState(const QUuid &master, const QUuid &slave);

signals:
    void started();
    void finished();
    void message(const QUuid &uuid, const QJsonObject &json);
    void remoteControl(const QUuid &master, const QUuid &slave);
    void controlStateChanged(SharedCursor::ControlState state);

private:
    int timerId = 0;
    SharedCursor::ControlState controlState = SharedCursor::SelfControl;
    SharedCursor::ConnectionState currentTransitState = SharedCursor::Unknown;
    QUuid transitUuid;
    QUuid ownUuid;
    QUuid controlledByUuid;
    QJsonObject jsonDelta, jsonPosition, jsonRemoteControl;
    QPoint lastCursorPosition = {0, 0};
    QPoint holdCursorPosition = {0, 0};
    QVector<SharedCursor::Transit> currentTransits;
    QMap<QUuid, QSharedPointer<SharedCursor::Device>> devices;
    QMap<QUuid, SharedCursor::ConnectionState> connnetionStates;

    void timerEvent(QTimerEvent *e);
    void checkCursor(const QPoint &pos);
    void sendCursorDelta(const QUuid &uuid, const QPoint &pos);
    void sendCursorPosition(const QUuid &uuid, const QPoint &pos);
    void sendCursorMessage(const QUuid &uuid, const char* type, const QPoint &pos);
    void setCursorPosition(const QPoint &pos);
    void sendRemoteControlMessage(bool state, const QPoint &pos);
    void updateControlState(SharedCursor::ControlState state);

    QPoint calculateRemotePos(const SharedCursor::Transit &transit, const QPoint &pos);
};
