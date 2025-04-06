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
    ownUuid = uuid;
    controlledByUuid = uuid;
    connnetionStates[uuid] = SharedCursor::Connected;

    auto it = devices.find(uuid);
    if (it != devices.end()) {
        currentDevice = it.value();
    }
    else {
        currentDevice.clear();
    }
}

void CursorHandler::setDevices(const QMap<QUuid, QSharedPointer<SharedCursor::Device> > &_devices)
{
    qDebug() << Q_FUNC_INFO;

    devices = _devices;
    currentDevice = _devices.value(ownUuid);
}

void CursorHandler::setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state)
{
    connnetionStates[uuid] = state;

    switch (controlState) {
    case SharedCursor::SelfControl:
        break;
    case SharedCursor::Master:
        if (uuid == transitUuid && state != SharedCursor::Connected) {
            updateControlState(SharedCursor::SelfControl);
            transitUuid = ownUuid;
            currentDevice = devices.value(transitUuid);
            emit remoteControl(ownUuid, ownUuid);
        }
        break;
    case SharedCursor::Slave:
        if (uuid == controlledByUuid && state != SharedCursor::Connected) {
            updateControlState(SharedCursor::SelfControl);
            transitUuid = ownUuid;
            currentDevice = devices.value(transitUuid);
        }
        break;
    }

    qDebug() << Q_FUNC_INFO << uuid << state << controlState;
}

void CursorHandler::setRemoteCursorPos(const QPoint &pos)
{
    if (controlState == SharedCursor::Master) {
        checkCursor(pos);
    }
}

void CursorHandler::setRemoteControlState(const QUuid &master, const QUuid &slave)
{
    controlledByUuid = master;

    if (master != slave) {
        if (ownUuid == master) updateControlState(SharedCursor::Master);
        else if(ownUuid == slave) updateControlState(SharedCursor::Slave);
        else updateControlState(SharedCursor::SelfControl);
    }
    else {
        updateControlState(SharedCursor::SelfControl);
        setCursorPosition(holdCursorPosition);
    }

    qDebug() << Q_FUNC_INFO << master << slave << controlState;
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
        sendCursorMessage(transitUuid, SharedCursor::KEY_CURSOR_DELTA, pos - holdCursorPosition);
        break;
    case SharedCursor::Slave:
        sendCursorMessage(controlledByUuid,SharedCursor::KEY_CURSOR_POS, pos);
        break;
    }

    lastCursorPosition = pos;
}

void CursorHandler::checkCursor(const QPoint &pos)
{
    if (currentDevice.isNull())
        return;

    if (currentDevice->transits.isEmpty())
        return;

    // point belongs to the line
    QUuid newTransitUuid = transitUuid;
    auto transitIterator = currentDevice->transits.begin();
    for (; transitIterator<currentDevice->transits.end(); ++transitIterator) {
        const QLine &line = transitIterator->line;
        if (line.x1() <= pos.x() && line.x2() >= pos.x() &&
            line.y1() <= pos.y() && line.y2() >= pos.y()) {
            if (newTransitUuid != transitIterator->uuid) {
                newTransitUuid = transitIterator->uuid;
            }
            break;
        }
    }

    // point just crossed the disabled screen
    auto screens = currentDevice->screens;
    for (auto screenIt=screens.begin(); screenIt<screens.end(); ++screenIt) {
        if (screenIt->enabled)
            continue;

        if (!screenIt->rect.contains(pos))
            continue;

        transitIterator = currentDevice->transits.begin();
        QLineF lineF(pos, lastCheckedCursorPosition);

        for (; transitIterator<currentDevice->transits.end(); ++transitIterator) {
            if (!lineF.intersects(transitIterator->line.toLineF()))
                continue;

            if (newTransitUuid != transitIterator->uuid) {
                newTransitUuid = transitIterator->uuid;
            }
            break;
        }

        if (transitIterator != currentDevice->transits.end())
            break;
    }

    lastCheckedCursorPosition = pos;

    if (newTransitUuid == transitUuid)
        return;

    cursorCrossedTransit(*transitIterator, pos);
}

void CursorHandler::cursorCrossedTransit(const SharedCursor::Transit &transit, const QPoint &pos)
{
    QUuid newTransitUuid = transit.uuid;

    if (connnetionStates.value(newTransitUuid) != SharedCursor::Connected )
        return;

    if (!devices.contains(newTransitUuid))
        return;

    transitUuid = newTransitUuid;
    currentTransitState = connnetionStates.value(transitUuid);
    currentDevice = devices.value(transitUuid);

    const QPoint remotePos = calculateRemotePos(transit, pos);
    emit remoteControl(ownUuid, transitUuid);
    sendCursorMessage(transitUuid, SharedCursor::KEY_INIT_CURSOR_POS, remotePos);

    if (transitUuid == ownUuid) {
        updateControlState(SharedCursor::SelfControl);
        setCursorPosition(remotePos);
    }
    else {
        updateControlState(SharedCursor::Master);
        setCursorPosition(holdCursorPosition);
    }

    qDebug() << Q_FUNC_INFO << transitUuid << controlState;
}

void CursorHandler::sendCursorMessage(const QUuid &uuid, const char *type, const QPoint &pos)
{
    if (pos == lastCursorPosition)
        return;

    jsonPosition[SharedCursor::KEY_TYPE] = type;
    jsonPosition[SharedCursor::KEY_VALUE] = SharedCursor::pointToJsonValue(pos);
    emit message(uuid, jsonPosition);
}

void CursorHandler::setCursorPosition(const QPoint &pos)
{
    QCursor::setPos(pos);
}

void CursorHandler::updateControlState(SharedCursor::ControlState state)
{
    if (controlState != state) {
        controlState = state;
        emit controlStateChanged(state);
    }
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
