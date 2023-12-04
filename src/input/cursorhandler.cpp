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
    connnetionStates[uuid] = SharedCursor::Connected;

    auto it = transitsMap.find(uuid);
    if (it != transitsMap.end()) {
        currentTransits = it.value();
    }
    else {
        currentTransits.clear();
    }
}

void CursorHandler::setTransits(const QMap<QUuid, QVector<SharedCursor::Transit> > &_transits)
{
    qDebug() << Q_FUNC_INFO;
    transitsMap = _transits;
    currentTransits = transitsMap.value(currentDeviceUuid);
}

void CursorHandler::setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state)
{
    qDebug() << Q_FUNC_INFO << uuid << state;
    connnetionStates[uuid] = state;

    switch (controlState) {
    case SharedCursor::SelfControl:
        break;
    case SharedCursor::Master:
        if (uuid == transitUuid && state != SharedCursor::Connected) {
            controlState = SharedCursor::SelfControl;
            transitUuid = currentDeviceUuid;
            currentTransits = transitsMap.value(transitUuid);
            sendRemoteControlMessage(false, {0, 0});
            emit controlRemoteDevice(currentDeviceUuid, false);
        }
        break;
    case SharedCursor::Slave:
        if (uuid == controlledByUuid && state != SharedCursor::Connected) {
            controlState = SharedCursor::SelfControl;
            transitUuid = currentDeviceUuid;
            currentTransits = transitsMap.value(transitUuid);
        }
        break;
    }
}

void CursorHandler::setRemoteCursorPos(const QUuid &uuid, const QPoint &pos)
{
    if (controlState == SharedCursor::Master && uuid == transitUuid) {
        checkCursor(pos);
    }
}

void CursorHandler::setControlledByUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;

    controlledByUuid = uuid;
    controlState = uuid == currentDeviceUuid ? SharedCursor::SelfControl : SharedCursor::Slave;
}

void CursorHandler::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId)
        return;

    const QPoint &pos = QCursor::pos();

    switch (controlState) {
    case SharedCursor::SelfControl:
        checkCursor(pos);
        break;
    case SharedCursor::Master:
        setCursorPosition(holdCursorPosition);
        sendCursorDelta(pos);
        break;
    case SharedCursor::Slave:
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
        if (connnetionStates.value(newTransitUuid) == SharedCursor::Connected && transitsMap.contains(newTransitUuid)) {
            sendRemoteControlMessage(false, {0, 0});
            transitUuid = newTransitUuid;
            currentTransitState = connnetionStates.value(transitUuid);
            currentTransits = transitsMap.value(transitUuid);

            const QPoint remotePos = calculateRemotePos(*transitIterator, pos);
            sendRemoteControlMessage(true, remotePos);

            if (transitUuid == currentDeviceUuid) {
                controlState = SharedCursor::SelfControl;
                emit controlRemoteDevice(transitUuid, false);
                setCursorPosition(remotePos);
            }
            else {
                controlState = SharedCursor::Master;
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

    jsonDelta[SharedCursor::KEY_TYPE] = SharedCursor::KEY_CURSOR_DELTA;
    jsonDelta[SharedCursor::KEY_CURSOR_DELTA] = SharedCursor::pointToJsonValue(pos - holdCursorPosition);
    emit message(transitUuid, jsonDelta);
}

void CursorHandler::sendCursorPosition(const QPoint &pos)
{
    if (pos == lastCursorPosition)
        return;

    jsonPosition[SharedCursor::KEY_TYPE] = SharedCursor::KEY_CURSOR_POS;
    jsonPosition[SharedCursor::KEY_CURSOR_POS] = SharedCursor::pointToJsonValue(pos);
    emit message(controlledByUuid, jsonPosition);
}

void CursorHandler::setCursorPosition(const QPoint &pos)
{
    QCursor::setPos(pos);
}

void CursorHandler::sendRemoteControlMessage(bool state, const QPoint &pos)
{
    jsonRemoteControl[SharedCursor::KEY_TYPE] = SharedCursor::KEY_REMOTE_CONTROL;
    jsonRemoteControl[SharedCursor::KEY_STATE] = state;
    jsonRemoteControl[SharedCursor::KEY_CURSOR_POS] = SharedCursor::pointToJsonValue(pos);
    emit message(transitUuid, jsonRemoteControl);
}

QPoint CursorHandler::calculateRemotePos(const SharedCursor::Transit &transit, const QPoint &pos)
{
    const QLine &line = transit.line;
    QPoint remoteCursorPosition(transit.pos.x(), transit.pos.y() + (pos.y() - line.y1()));

    if (line.y1() == line.y2()) {
        remoteCursorPosition = {transit.pos.x() + (pos.x() - line.x1()), transit.pos.y()};
    }

    return remoteCursorPosition;
}
