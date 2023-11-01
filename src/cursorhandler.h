#pragma once

#include <QObject>

class CursorHandler : public QObject
{
    Q_OBJECT
public:
    explicit CursorHandler(QObject *parent = nullptr);
    ~CursorHandler();

public slots:
    void start();
    void stop();

signals:
    void started();
    void finished();

private:
    int timerId = 0;

    void timerEvent(QTimerEvent *e);
    void checkCursor();

    bool isLeftButtonPressed();
};
