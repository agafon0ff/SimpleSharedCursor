#pragma once

#include <QJsonObject>
#include <QWidget>
#include <QEvent>
#include <QUuid>

class InputHandler : public QWidget
{
    Q_OBJECT
public:
    explicit InputHandler(QWidget *parent = nullptr);

signals:
    void message(const QUuid &uuid, const QJsonObject &json);

public slots:
    void setUuid(const QUuid &uuid);
    void setCenterIn(const QPoint &pos);
    void setRemoteControlState(const QUuid &master, const QUuid &slave);

private:
    bool isActive = false;
    QUuid ownUuid, remoteUuid;
    QJsonObject jsonKey;
    QVector<int> pressedKeys;

    void paintEvent(QPaintEvent *e);

    bool event(QEvent *event) override;
    void sendKeyEventMessage(int keycode, bool pressed);
    void keyStateChanged(QKeyEvent *event, bool pressed);
    void mouseStateChanged(QMouseEvent *event, bool pressed);
    void wheelStateChanged(QWheelEvent *event);
};
