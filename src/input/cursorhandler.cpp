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
    currentTransits = transitsMap.value(ownUuid);
}

void CursorHandler::setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state)
{
    connnetionStates[uuid] = state;

    switch (controlState) {
    case SharedCursor::SelfControl:
        break;
    case SharedCursor::Master:
        if (uuid == transitUuid && state != SharedCursor::Connected) {
            controlState = SharedCursor::SelfControl;
            transitUuid = ownUuid;
            currentTransits = transitsMap.value(transitUuid);
            emit remoteControl(ownUuid, ownUuid);
        }
        break;
    case SharedCursor::Slave:
        if (uuid == controlledByUuid && state != SharedCursor::Connected) {
            controlState = SharedCursor::SelfControl;
            transitUuid = ownUuid;
            currentTransits = transitsMap.value(transitUuid);
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
        if (ownUuid == master) controlState = SharedCursor::Master;
        else if(ownUuid == slave) controlState = SharedCursor::Slave;
    }
    else {
        controlState = SharedCursor::SelfControl;
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
            transitUuid = newTransitUuid;
            currentTransitState = connnetionStates.value(transitUuid);
            currentTransits = transitsMap.value(transitUuid);

            const QPoint remotePos = calculateRemotePos(*transitIterator, pos);
            emit remoteControl(ownUuid, transitUuid);
            sendCursorMessage(transitUuid, SharedCursor::KEY_INIT_CURSOR_POS, remotePos);

            if (transitUuid == ownUuid) {
                controlState = SharedCursor::SelfControl;
                setCursorPosition(remotePos);
            }
            else {
                controlState = SharedCursor::Master;
                setCursorPosition(holdCursorPosition);
            }

            qDebug() << Q_FUNC_INFO << transitUuid << controlState;
        }
    }
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

QPoint CursorHandler::calculateRemotePos(const SharedCursor::Transit &transit, const QPoint &pos)
{
    const QLine &line = transit.line;
    QPoint remoteCursorPosition(transit.pos.x(), transit.pos.y() + (pos.y() - line.y1()));

    if (line.y1() == line.y2()) {
        remoteCursorPosition = {transit.pos.x() + (pos.x() - line.x1()), transit.pos.y()};
    }

    return remoteCursorPosition;
}
