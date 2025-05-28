#include <QTimerEvent>
#include <QDebug>
#include <QCursor>
#include <qmath.h>

#include "cursorhandler.h"
#include "utils.h"

static const int UPDATE_INTERVAL = 25;

CursorHandler::CursorHandler(QObject *parent)
    : QObject{parent}
{
    qDebug() << Q_FUNC_INFO;
}

CursorHandler::~CursorHandler()
{
    qDebug() << Q_FUNC_INFO;

    if (_timerId)
        killTimer(_timerId);
}

void CursorHandler::start()
{
    qDebug() << Q_FUNC_INFO;

    _timerId = startTimer(UPDATE_INTERVAL);
    emit started();
}

void CursorHandler::stop()
{
    qDebug() << Q_FUNC_INFO;

    if (_timerId) {
        killTimer(_timerId);
        _timerId = 0;
    }

    emit finished();
}

void CursorHandler::setHoldCursorPosition(const QPoint &pos)
{
    qDebug() << Q_FUNC_INFO << pos;
    _holdCursorPosition = pos;
}

void CursorHandler::setCurrentUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;

    _transitUuid = uuid;
    _ownUuid = uuid;
    _controlledByUuid = uuid;
    _connnetionStates[uuid] = SharedCursor::Connected;

    auto it = _devices.find(uuid);
    if (it != _devices.end()) {
        _currentDevice = it.value();
    }
    else {
        _currentDevice.clear();
    }
}

void CursorHandler::setDevices(const QMap<QUuid, QSharedPointer<SharedCursor::Device> > &devices)
{
    qDebug() << Q_FUNC_INFO;

    _devices = devices;
    _currentDevice = devices.value(_ownUuid);
}

void CursorHandler::setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state)
{
    _connnetionStates[uuid] = state;

    switch (_controlState) {
    case SharedCursor::SelfControl:
        break;
    case SharedCursor::Master:
        if (uuid == _transitUuid && state != SharedCursor::Connected) {
            updateControlState(SharedCursor::SelfControl);
            _transitUuid = _ownUuid;
            _currentDevice = _devices.value(_transitUuid);
            emit remoteControl(_ownUuid, _ownUuid);
        }
        break;
    case SharedCursor::Slave:
        if (uuid == _controlledByUuid && state != SharedCursor::Connected) {
            updateControlState(SharedCursor::SelfControl);
            _transitUuid = _ownUuid;
            _currentDevice = _devices.value(_transitUuid);
        }
        break;
    }

    qDebug() << Q_FUNC_INFO << uuid << state << _controlState;
}

void CursorHandler::setRemoteCursorDelta(const QPoint &pos)
{
    Q_UNUSED(pos);
    _lastRemoteCursorTime = QDateTime::currentDateTime();
}

void CursorHandler::setRemoteCursorPos(const QPoint &pos)
{
    if (_controlState == SharedCursor::Master)
        checkCursor(pos);
}

void CursorHandler::setRemoteControlState(const QUuid &master, const QUuid &slave)
{
    _controlledByUuid = master;

    if (master != slave) {
        if (_ownUuid == master) updateControlState(SharedCursor::Master);
        else if(_ownUuid == slave) updateControlState(SharedCursor::Slave);
        else updateControlState(SharedCursor::SelfControl);
    }
    else {
        updateControlState(SharedCursor::SelfControl);
        setCursorPosition(_holdCursorPosition);
    }

    qDebug() << Q_FUNC_INFO << master << slave << _controlState;
}

void CursorHandler::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != _timerId)
        return;

    const QPoint &pos = QCursor::pos();

    switch (_controlState) {
    case SharedCursor::SelfControl:
        checkCursor(pos);
        break;
    case SharedCursor::Master:
        if (pos == _lastCursorPosition)
            break;

        setCursorPosition(_holdCursorPosition);
        sendCursorMessage(_transitUuid, SharedCursor::KEY_CURSOR_DELTA, pos - _holdCursorPosition);
        break;
    case SharedCursor::Slave:
        sendCursorMessage(_controlledByUuid,SharedCursor::KEY_CURSOR_POS, pos);
        checkSelfControlInSlaveMode(pos);
        break;
    }

    _lastCursorPosition = pos;
}

void CursorHandler::checkCursor(const QPoint &pos)
{
    if (_currentDevice.isNull())
        return;

    if (_currentDevice->transits.isEmpty())
        return;

    // point belongs to the line
    QUuid newTransitUuid = _transitUuid;
    auto transitIterator = _currentDevice->transits.begin();
    for (; transitIterator<_currentDevice->transits.end(); ++transitIterator) {
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
    auto screens = _currentDevice->screens;
    for (auto screenIt=screens.begin(); screenIt<screens.end(); ++screenIt) {
        if (screenIt->enabled)
            continue;

        if (!screenIt->rect.contains(pos))
            continue;

        transitIterator = _currentDevice->transits.begin();
        QLineF lineF(pos, _lastCheckedCursorPosition);

        for (; transitIterator<_currentDevice->transits.end(); ++transitIterator) {
            if (!lineF.intersects(transitIterator->line.toLineF()))
                continue;

            if (newTransitUuid != transitIterator->uuid) {
                newTransitUuid = transitIterator->uuid;
            }
            break;
        }

        if (transitIterator != _currentDevice->transits.end())
            break;
    }

    _lastCheckedCursorPosition = pos;

    if (newTransitUuid == _transitUuid)
        return;

    cursorCrossedTransit(*transitIterator, pos);
}

void CursorHandler::checkSelfControlInSlaveMode(const QPoint &pos)
{
    if (_lastRemoteCursorTime.addMSecs(1000) < QDateTime::currentDateTime() &&
        pos != _lastCursorPosition)
    {
        ++_selfCotrolInSlaveModeCounter;
    }
    else
    {
        _selfCotrolInSlaveModeCounter = 0;
    }

    if (_selfCotrolInSlaveModeCounter > 10)
    {
        _transitUuid = _ownUuid;
        updateControlState(SharedCursor::SelfControl);
        emit remoteControl(_ownUuid, _transitUuid);
        _selfCotrolInSlaveModeCounter = 0;

        qDebug() << Q_FUNC_INFO << _transitUuid << _controlState;
    }
}

void CursorHandler::cursorCrossedTransit(const SharedCursor::Transit &transit, const QPoint &pos)
{
    QUuid newTransitUuid = transit.uuid;

    if (_connnetionStates.value(newTransitUuid) != SharedCursor::Connected )
        return;

    if (!_devices.contains(newTransitUuid))
        return;

    _transitUuid = newTransitUuid;
    _currentTransitState = _connnetionStates.value(_transitUuid);
    _currentDevice = _devices.value(_transitUuid);

    const QPoint remotePos = calculateRemotePos(transit, pos);
    emit remoteControl(_ownUuid, _transitUuid);
    sendCursorMessage(_transitUuid, SharedCursor::KEY_INIT_CURSOR_POS, remotePos);

    if (_transitUuid == _ownUuid) {
        updateControlState(SharedCursor::SelfControl);
        setCursorPosition(remotePos);
    }
    else {
        updateControlState(SharedCursor::Master);
        setCursorPosition(_holdCursorPosition);
    }

    qDebug() << Q_FUNC_INFO << _transitUuid << _controlState;
}

void CursorHandler::sendCursorMessage(const QUuid &uuid, const char *type, const QPoint &pos)
{
    if (pos == _lastCursorPosition)
        return;

    _jsonPosition[SharedCursor::KEY_TYPE] = type;
    _jsonPosition[SharedCursor::KEY_VALUE] = SharedCursor::pointToJsonValue(pos);
    emit message(uuid, _jsonPosition);
}

void CursorHandler::setCursorPosition(const QPoint &pos)
{
    QCursor::setPos(pos);
}

void CursorHandler::updateControlState(SharedCursor::ControlState state)
{
    if (_controlState != state) {
        _controlState = state;
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
