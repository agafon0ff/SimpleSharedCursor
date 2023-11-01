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
    timerId = startTimer(UPDATE_INTERVAL);
    emit started();
}

void CursorHandler::stop()
{
    if (timerId) {
        killTimer(timerId);
        timerId = 0;
    }

    emit finished();
}

void CursorHandler::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId)
        return;

    checkCursor();
}

void CursorHandler::checkCursor()
{
//    qDebug() << QCursor::pos();
}
