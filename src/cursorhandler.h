#pragma once

#include <QObject>
#include <QSharedPointer>
#include "global.h"

class CursorHandler : public QObject
{
    Q_OBJECT
public:
    explicit CursorHandler(QObject *parent = nullptr);
    ~CursorHandler();

public slots:
    void start();
    void stop();

    void setCurrentUuid(const QUuid &uuid);
    void setTransits(const QMap<QUuid, QVector<ShareCursor::Transit>> &transits);
    void setConnectionState(const QUuid &uuid, ShareCursor::ConnectionState state);

signals:
    void started();
    void finished();

private:
    int timerId = 0;
    QMap<QUuid, QVector<ShareCursor::Transit>> transitsMap;
    QVector<ShareCursor::Transit> currentTransits;
    QMap<QUuid, ShareCursor::ConnectionState> connnetionStates;
    QUuid transitUuid;
    QUuid currentUuid;
    ShareCursor::ConnectionState currentTransitState = ShareCursor::Unknown;

    void timerEvent(QTimerEvent *e);
    void checkCursor();

    bool isLeftButtonPressed();
};
