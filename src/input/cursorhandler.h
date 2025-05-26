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
    int _timerId = 0;
    SharedCursor::ControlState _controlState = SharedCursor::SelfControl;
    SharedCursor::ConnectionState _currentTransitState = SharedCursor::Unknown;
    QUuid _transitUuid;
    QUuid _ownUuid;
    QUuid _controlledByUuid;
    QJsonObject _jsonDelta, _jsonPosition, _jsonRemoteControl;
    QPoint _lastCursorPosition = {0, 0};
    QPoint _holdCursorPosition = {0, 0};
    QPoint _lastCheckedCursorPosition = {0, 0};
    bool _deviceContainsDisableScreen = false;
    QSharedPointer<SharedCursor::Device> _currentDevice;
    QMap<QUuid, QSharedPointer<SharedCursor::Device>> _devices;
    QMap<QUuid, SharedCursor::ConnectionState> _connnetionStates;

    void timerEvent(QTimerEvent *e) final;
    void checkCursor(const QPoint &pos);
    void cursorCrossedTransit(const SharedCursor::Transit &transit, const QPoint &pos);
    void sendCursorDelta(const QUuid &uuid, const QPoint &pos);
    void sendCursorPosition(const QUuid &uuid, const QPoint &pos);
    void sendCursorMessage(const QUuid &uuid, const char* type, const QPoint &pos);
    void setCursorPosition(const QPoint &pos);
    void sendRemoteControlMessage(bool state, const QPoint &pos);
    void updateControlState(SharedCursor::ControlState state);

    QPoint calculateRemotePos(const SharedCursor::Transit &transit, const QPoint &pos);
};
