#include <QTimerEvent>
#include <QDebug>
#include <QCursor>
#include <qmath.h>

#include "cursorhandler.h"

static const int UPDATE_INTERVAL = 25;

CursorHandler::CursorHandler(QObject *parent)
    : QObject{parent}
{
    qDebug() << Q_FUNC_INFO;
}

CursorHandler::~CursorHandler()
{
    qDebug() << Q_FUNC_INFO;

    if (timerId)
        killTimer(timerId);
}

void CursorHandler::start()
{
    qDebug() << Q_FUNC_INFO;

    timerId = startTimer(UPDATE_INTERVAL);
    emit started();
}

void CursorHandler::stop()
{
    qDebug() << Q_FUNC_INFO;

    if (timerId) {
        killTimer(timerId);
        timerId = 0;
    }

    emit finished();
}

void CursorHandler::setHoldCursorPosition(const QPoint &pos)
{
    qDebug() << Q_FUNC_INFO << pos;
    holdCursorPosition = pos;
}

void CursorHandler::setCurrentUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;

    transitUuid = uuid;
    currentDeviceUuid = uuid;
    controlledByUuid = uuid;
    connnetionStates[uuid] = ShareCursor::Connected;

    auto it = transitsMap.find(uuid);
    if (it != transitsMap.end()) {
        currentTransits = it.value();
    }
    else {
        currentTransits.clear();
    }
}

void CursorHandler::setTransits(const QMap<QUuid, QVector<ShareCursor::Transit> > &_transits)
{
    qDebug() << Q_FUNC_INFO;
    transitsMap = _transits;
    currentTransits = transitsMap.value(currentDeviceUuid);
}

void CursorHandler::setConnectionState(const QUuid &uuid, ShareCursor::ConnectionState state)
{
    qDebug() << Q_FUNC_INFO << uuid << state;
    connnetionStates[uuid] = state;

    switch (controlState) {
    case ShareCursor::SelfControl:
        break;
    case ShareCursor::Master:
        if (uuid == transitUuid && state != ShareCursor::Connected) {
            controlState = ShareCursor::SelfControl;
            transitUuid = currentDeviceUuid;
            sendRemoteControlMessage(false, {0, 0});
            emit controlRemoteDevice(currentDeviceUuid, false);
        }
        break;
    case ShareCursor::Slave:
        if (uuid == controlledByUuid && state != ShareCursor::Connected) {
            controlState = ShareCursor::SelfControl;
            transitUuid = currentDeviceUuid;
        }
        break;
    }
}

void CursorHandler::setRemoteCursorPos(const QUuid &uuid, const QPoint &pos)
{
    if (controlState == ShareCursor::Master && uuid == transitUuid) {
        checkCursor(pos);
    }
}

void CursorHandler::setControlledByUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;

    controlledByUuid = uuid;
    controlState = uuid == currentDeviceUuid ? ShareCursor::SelfControl : ShareCursor::Slave;
}

void CursorHandler::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId)
        return;

    const QPoint &pos = QCursor::pos();

    switch (controlState) {
    case ShareCursor::SelfControl:
        checkCursor(pos);
        break;
    case ShareCursor::Master:
        setCursorPosition(holdCursorPosition);
        sendCursorDelta(pos);
        break;
    case ShareCursor::Slave:
        sendCursorPosition(pos);
        break;
    }

    lastCursorPosition = pos;
}

void CursorHandler::checkCursor(const QPoint &pos)
{
    if (currentTransits.isEmpty()) {
        return;
    }

    QUuid newTransitUuid = transitUuid;
    auto transitIterator = currentTransits.begin();

    for (; transitIterator<currentTransits.end(); ++transitIterator) {
        const QLine &line = transitIterator->line;
        if (line.x1() <= pos.x() && line.x2() >= pos.x() &&
            line.y1() <= pos.y() && line.y2() >= pos.y()) {
            if (newTransitUuid != transitIterator->uuid) {
                newTransitUuid = transitIterator->uuid;
            }
            break;
        }
    }

    if (newTransitUuid != transitUuid) {
        if (connnetionStates.value(newTransitUuid) == ShareCursor::Connected && transitsMap.contains(newTransitUuid)) {
            sendRemoteControlMessage(false, {0, 0});
            transitUuid = newTransitUuid;
            currentTransitState = connnetionStates.value(transitUuid);
            currentTransits = transitsMap.value(transitUuid);

            const QPoint remotePos = calculateRemotePos(*transitIterator, pos);
            sendRemoteControlMessage(true, remotePos);

            if (transitUuid == currentDeviceUuid) {
                controlState = ShareCursor::SelfControl;
                emit controlRemoteDevice(transitUuid, false);
                setCursorPosition(remotePos);
            }
            else {
                controlState = ShareCursor::Master;
                emit controlRemoteDevice(transitUuid, true);
                setCursorPosition(holdCursorPosition);
            }

            qDebug() << Q_FUNC_INFO << transitUuid;
        }
    }
}

void CursorHandler::sendCursorDelta(const QPoint &pos)
{
    if (pos == lastCursorPosition)
        return;

    jsonDelta[ShareCursor::KEY_TYPE] = ShareCursor::KEY_CURSOR_DELTA;
    jsonDelta[ShareCursor::KEY_CURSOR_DELTA] = ShareCursor::pointToJsonValue(pos - holdCursorPosition);
    emit message(transitUuid, jsonDelta);
}

void CursorHandler::sendCursorPosition(const QPoint &pos)
{
    if (pos == lastCursorPosition)
        return;

    jsonPosition[ShareCursor::KEY_TYPE] = ShareCursor::KEY_CURSOR_POS;
    jsonPosition[ShareCursor::KEY_CURSOR_POS] = ShareCursor::pointToJsonValue(pos);
    emit message(controlledByUuid, jsonPosition);
}

void CursorHandler::setCursorPosition(const QPoint &pos)
{
    QCursor::setPos(pos);
}

void CursorHandler::sendRemoteControlMessage(bool state, const QPoint &pos)
{
    jsonRemoteControl[ShareCursor::KEY_TYPE] = ShareCursor::KEY_REMOTE_CONTROL;
    jsonRemoteControl[ShareCursor::KEY_STATE] = state;
    jsonRemoteControl[ShareCursor::KEY_CURSOR_POS] = ShareCursor::pointToJsonValue(pos);
    emit message(transitUuid, jsonRemoteControl);
}

QPoint CursorHandler::calculateRemotePos(const ShareCursor::Transit &transit, const QPoint &pos)
{
    const QLine &line = transit.line;
    QPoint remoteCursorPosition(transit.pos.x(), transit.pos.y() + (pos.y() - line.y1()));

    if (line.y1() == line.y2()) {
        remoteCursorPosition = {transit.pos.x() + (pos.x() - line.x1()), transit.pos.y()};
    }

    return remoteCursorPosition;
}
