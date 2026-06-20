#pragma once

#include <QObject>
#include <memory>

#include "global.h"

class InputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit InputSimulator(QObject *parent = nullptr);
    virtual ~InputSimulator();
    static std::unique_ptr<InputSimulator> create();

public slots:
    virtual void setControlState(SharedCursor::ControlState state) = 0;
    virtual void setCursorPosition(const QPoint &pos) = 0;
    virtual void setCursorDelta(const QPoint &pos) = 0;
    virtual void setKeyboardEvent(int keycode, bool state) = 0;
    virtual void setMouseEvent(int button, bool state) = 0;
    virtual void setWheelEvent(int delta) = 0;
};

using InputSimulatorPtr = std::unique_ptr<InputSimulator>;
