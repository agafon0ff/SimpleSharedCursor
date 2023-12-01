#pragma once

#include <QJsonObject>
#include <QWidget>
#include <QEvent>
#include <QUuid>
#include "utils.h"

class InputHandler : public QWidget
{
    Q_OBJECT
public:
    explicit InputHandler(QWidget *parent = nullptr);

signals:
    void message(const QUuid &uuid, const QJsonObject &json);

public slots:
    void holdCursor(const QUuid &uuid, bool state);
    void setCenterIn(const QPoint &pos);

private:
    bool isActive = false;
    QUuid remoteUuid;
    QJsonObject jsonKey;

    bool event(QEvent *event) override;
    void keyStateChanged(QKeyEvent *event, bool pressed);
    void mouseStateChanged(QMouseEvent *event, bool pressed);
    void wheelStateChanged(QWheelEvent *event);
};
