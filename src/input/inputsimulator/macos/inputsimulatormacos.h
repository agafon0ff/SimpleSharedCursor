#pragma once

#include "inputsimulator.h"
#include <QMap>
#include <QVector>

class InputSimulatorMacos final : public InputSimulator
{
    Q_OBJECT
public:
    explicit InputSimulatorMacos(QObject *parent = nullptr);
    ~InputSimulatorMacos();

public slots:
    void setControlState(SharedCursor::ControlState state) final;
    void setCursorPosition(const QPoint &pos) final;
    void setCursorDelta(const QPoint &pos) final;
    void setKeyboardEvent(int keycode, bool state) final;
    void setMouseEvent(int button, bool state) final;
    void setWheelEvent(int delta) final;

private:
    QMap<int, unsigned short> _keymap;
    SharedCursor::ControlState _controlState = SharedCursor::SelfControl;
    bool _releaseProcess = false;
    QVector<int> _pressedKeys;
    QVector<int> _pressedMouse;
    void releasePressedKeys();
    void createKeymap();
    void checkAccessibilityPermission();
    void postMouseMove(const QPoint &pos);
};
