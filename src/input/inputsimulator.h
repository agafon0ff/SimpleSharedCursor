#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

#include "global.h"

class InputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit InputSimulator(QObject *parent = nullptr);

public slots:
    void setControlState(SharedCursor::ControlState state);
    void setCutsorPosition(const QPoint &pos);
    void setCutsorDelta(const QPoint &pos);
    void setKeyboardEvent(int keycode, bool state);
    void setMouseEvent(int button, bool state);
    void setWheelEvent(int delta);

private:
    QMap<int, unsigned long> keymap;
    SharedCursor::ControlState controlState = SharedCursor::SelfControl;

    bool releaseProcess = false;
    QVector<int> pressedKeys;
    QVector<int> pressedMouse;

    void releasePressedKeys();
    void createKeymap();
};
