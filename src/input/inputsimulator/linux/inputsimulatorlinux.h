#pragma once

#include <QVector>
#include <QMap>

#include "inputsimulator.h"

struct _XDisplay;

class InputSimulatorLinux final : public InputSimulator
{
    Q_OBJECT
public:
    explicit InputSimulatorLinux(QObject *parent = nullptr);
    ~InputSimulatorLinux();

public slots:
    void setControlState(SharedCursor::ControlState state) final;
    void setCursorPosition(const QPoint &pos) final;
    void setCursorDelta(const QPoint &pos) final;
    void setKeyboardEvent(int keycode, bool state) final;
    void setMouseEvent(int button, bool state) final;
    void setWheelEvent(int delta) final;

private:
    QMap<int, unsigned long> _keymap;
    SharedCursor::ControlState _controlState = SharedCursor::SelfControl;

    bool _releaseProcess = false;
    QVector<int> _pressedKeys;
    QVector<int> _pressedMouse;

    void releasePressedKeys();
    void createKeymap();

    _XDisplay *_display = nullptr;
};
