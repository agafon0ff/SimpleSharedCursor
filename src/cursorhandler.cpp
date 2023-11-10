#include <QTimerEvent>
#include <QDebug>
#include <QCursor>

#include "cursorhandler.h"

#ifdef WIN32
#endif

#ifdef __linux__
#endif

static const int UPDATE_INTERVAL = 50;

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

void CursorHandler::setCurrentUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;
    currentUuid = uuid;
}

void CursorHandler::setTransits(const QMap<QUuid, QVector<ShareCursor::Transit> > &_transits)
{
    qDebug() << Q_FUNC_INFO;
    transitsMap = _transits;
    currentTransits = transitsMap.value(currentUuid);
}

void CursorHandler::setConnectionState(const QUuid &uuid, ShareCursor::ConnectionState state)
{
    qDebug() << Q_FUNC_INFO << uuid << state;
    connnetionStates[uuid] = state;
}

void CursorHandler::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId)
        return;

    checkCursor();
}

void CursorHandler::checkCursor()
{
    QUuid newTransitUuid = currentUuid;
    const QPoint &pos = QCursor::pos();

    for (const ShareCursor::Transit &transit: qAsConst(currentTransits)) {
        const QLine &line = transit.line;
        if (line.x1() <= pos.x() && line.x2() >= pos.x() &&
            line.y1() <= pos.y() && line.y2() >= pos.y()) {
            if (newTransitUuid != transit.uuid) {
                newTransitUuid = transit.uuid;
            }
            break;
        }
    }

    if (newTransitUuid != transitUuid) {
        transitUuid = newTransitUuid;
        currentTransitState = connnetionStates.value(transitUuid);

        qDebug() << "Intersect with:" << newTransitUuid << ", state:" << currentTransitState;
    }
}
