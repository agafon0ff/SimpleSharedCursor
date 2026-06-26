#include <QCursor>
#include <QDebug>
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>
#include <QAbstractButton>
#include "inputsimulatormacos.h"

#include <ApplicationServices/ApplicationServices.h>

InputSimulatorMacos::InputSimulatorMacos(QObject *parent)
    : InputSimulator{parent}
{
    createKeymap();
    checkAccessibilityPermission();
}

InputSimulatorMacos::~InputSimulatorMacos()
{
}

void InputSimulatorMacos::setControlState(SharedCursor::ControlState state)
{
    if (_controlState != state) {
        if (_controlState == SharedCursor::Slave) {
            releasePressedKeys();
        }
        _controlState = state;
    }
}

void InputSimulatorMacos::setCursorPosition(const QPoint &pos)
{
    postMouseMove(pos);
}

void InputSimulatorMacos::setCursorDelta(const QPoint &pos)
{
    postMouseMove(QCursor::pos() + pos);
}

void InputSimulatorMacos::postMouseMove(const QPoint &pos)
{
    CGPoint cgPos = CGPointMake(pos.x(), pos.y());

    CGEventType eventType;
    CGMouseButton mouseButton = kCGMouseButtonLeft;

    if (_pressedMouse.contains(0)) {
        eventType = kCGEventLeftMouseDragged;
        mouseButton = kCGMouseButtonLeft;
    } else if (_pressedMouse.contains(2)) {
        eventType = kCGEventRightMouseDragged;
        mouseButton = kCGMouseButtonRight;
    } else if (_pressedMouse.contains(1)) {
        eventType = kCGEventOtherMouseDragged;
        mouseButton = kCGMouseButtonCenter;
    } else {
        eventType = kCGEventMouseMoved;
    }

    CGEventRef event = CGEventCreateMouseEvent(nullptr, eventType, cgPos, mouseButton);
    if (event) {
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}

void InputSimulatorMacos::setKeyboardEvent(int keycode, bool state)
{
    if (!_releaseProcess) {
        if (state) _pressedKeys.append(keycode);
        else _pressedKeys.removeOne(keycode);
    }

    unsigned short vk = 0;
    auto it = _keymap.find(keycode);
    if (it != _keymap.end()) {
        vk = it.value();
    } else {
        vk = static_cast<unsigned short>(keycode);
    }

    CGEventRef event = CGEventCreateKeyboardEvent(nullptr, vk, state);
    if (event) {
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}

void InputSimulatorMacos::setMouseEvent(int button, bool state)
{
    if (!_releaseProcess) {
        if (state) _pressedMouse.append(button);
        else _pressedMouse.removeOne(button);
    }

    QPoint p = QCursor::pos();
    CGPoint pos = CGPointMake(p.x(), p.y());

    CGEventType eventType;
    CGMouseButton mouseButton;

    if (button == 0) { // left
        eventType = state ? kCGEventLeftMouseDown : kCGEventLeftMouseUp;
        mouseButton = kCGMouseButtonLeft;
    } else if (button == 1) { // middle
        eventType = state ? kCGEventOtherMouseDown : kCGEventOtherMouseUp;
        mouseButton = kCGMouseButtonCenter;
    } else { // right
        eventType = state ? kCGEventRightMouseDown : kCGEventRightMouseUp;
        mouseButton = kCGMouseButtonRight;
    }

    CGEventRef event = CGEventCreateMouseEvent(nullptr, eventType, pos, mouseButton);
    if (event) {
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}

void InputSimulatorMacos::setWheelEvent(int delta)
{
    int lines = delta > 0 ? 1 : -1;
    CGEventRef event = CGEventCreateScrollWheelEvent(nullptr, kCGScrollEventUnitLine, 1, lines);
    if (event) {
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}

void InputSimulatorMacos::releasePressedKeys()
{
    _releaseProcess = true;

    if (!_pressedKeys.empty()) {
        for (int code : std::as_const(_pressedKeys)) {
            setKeyboardEvent(code, false);
        }
    }

    if (!_pressedMouse.empty()) {
        for (int button : std::as_const(_pressedMouse)) {
            setMouseEvent(button, false);
        }
    }

    _releaseProcess = false;
}

void InputSimulatorMacos::checkAccessibilityPermission()
{
    if (AXIsProcessTrusted())
        return;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Accessibility Permission Required");
    msgBox.setText("SimpleSharedCursor needs Accessibility permission to simulate keyboard and mouse input.\n\n"
                   "Please open System Settings and grant access under Privacy & Security → Accessibility.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.button(QMessageBox::Ok)->setText("Open Settings");

    if (msgBox.exec() == QMessageBox::Ok) {
        QDesktopServices::openUrl(QUrl("x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility"));
    }
}

void InputSimulatorMacos::createKeymap()
{
    _keymap = {
        { Qt::Key_Escape,      0x35 },
        { Qt::Key_Tab,         0x30 },
        { Qt::Key_Backspace,   0x33 },
        { Qt::Key_Return,      0x24 },
        { Qt::Key_Enter,       0x4C },
        { Qt::Key_Insert,      0x72 },
        { Qt::Key_Delete,      0x75 },
        { Qt::Key_Pause,       0x71 },
        { Qt::Key_Print,       0x69 },
        { Qt::Key_Home,        0x73 },
        { Qt::Key_End,         0x77 },
        { Qt::Key_Left,        0x7B },
        { Qt::Key_Up,          0x7E },
        { Qt::Key_Right,       0x7C },
        { Qt::Key_Down,        0x7D },
        { Qt::Key_PageUp,      0x74 },
        { Qt::Key_PageDown,    0x79 },
        { Qt::Key_Shift,       0x38 },
        { Qt::Key_Control,     0x3B },
        { Qt::Key_Meta,        0x37 },
        { Qt::Key_Alt,         0x3A },
        { Qt::Key_CapsLock,    0x39 },
        { Qt::Key_F1,          0x7A },
        { Qt::Key_F2,          0x78 },
        { Qt::Key_F3,          0x63 },
        { Qt::Key_F4,          0x76 },
        { Qt::Key_F5,          0x60 },
        { Qt::Key_F6,          0x61 },
        { Qt::Key_F7,          0x62 },
        { Qt::Key_F8,          0x64 },
        { Qt::Key_F9,          0x65 },
        { Qt::Key_F10,         0x6D },
        { Qt::Key_F11,         0x67 },
        { Qt::Key_F12,         0x6F },
        { Qt::Key_F13,         0x69 },
        { Qt::Key_F14,         0x6B },
        { Qt::Key_Space,       0x31 },
        { Qt::Key_0,           0x1D },
        { Qt::Key_1,           0x12 },
        { Qt::Key_2,           0x13 },
        { Qt::Key_3,           0x14 },
        { Qt::Key_4,           0x15 },
        { Qt::Key_5,           0x17 },
        { Qt::Key_6,           0x16 },
        { Qt::Key_7,           0x1A },
        { Qt::Key_8,           0x1C },
        { Qt::Key_9,           0x19 },
        { Qt::Key_A,           0x00 },
        { Qt::Key_B,           0x0B },
        { Qt::Key_C,           0x08 },
        { Qt::Key_D,           0x02 },
        { Qt::Key_E,           0x0E },
        { Qt::Key_F,           0x03 },
        { Qt::Key_G,           0x05 },
        { Qt::Key_H,           0x04 },
        { Qt::Key_I,           0x22 },
        { Qt::Key_J,           0x26 },
        { Qt::Key_K,           0x28 },
        { Qt::Key_L,           0x25 },
        { Qt::Key_M,           0x2E },
        { Qt::Key_N,           0x2D },
        { Qt::Key_O,           0x1F },
        { Qt::Key_P,           0x23 },
        { Qt::Key_Q,           0x0C },
        { Qt::Key_R,           0x0F },
        { Qt::Key_S,           0x01 },
        { Qt::Key_T,           0x11 },
        { Qt::Key_U,           0x20 },
        { Qt::Key_V,           0x09 },
        { Qt::Key_W,           0x0D },
        { Qt::Key_X,           0x07 },
        { Qt::Key_Y,           0x10 },
        { Qt::Key_Z,           0x06 },
        { Qt::Key_Minus,        0x1B },
        { Qt::Key_Equal,        0x18 },
        { Qt::Key_BracketLeft,  0x21 },
        { Qt::Key_BracketRight, 0x1E },
        { Qt::Key_Backslash,    0x2A },
        { Qt::Key_Semicolon,    0x29 },
        { Qt::Key_Apostrophe,   0x27 },
        { Qt::Key_QuoteLeft,    0x32 },
        { Qt::Key_Comma,        0x2B },
        { Qt::Key_Period,       0x2F },
        { Qt::Key_Slash,        0x2C },
        // shifted variants — Shift is sent as a separate key event,
        // so these map to the same physical key as their unshifted counterparts
        { Qt::Key_Exclam,       0x12 }, // Shift+1
        { Qt::Key_QuoteDbl,     0x27 }, // Shift+' (same physical key as apostrophe)
        { Qt::Key_NumberSign,   0x14 }, // Shift+3
        { Qt::Key_Dollar,       0x15 }, // Shift+4
        { Qt::Key_Percent,      0x17 }, // Shift+5
        { Qt::Key_Ampersand,    0x1A }, // Shift+7
        { Qt::Key_ParenLeft,    0x19 }, // Shift+9
        { Qt::Key_ParenRight,   0x1D }, // Shift+0
        { Qt::Key_Asterisk,     0x1C }, // Shift+8
        { Qt::Key_Plus,         0x18 }, // Shift+= (same physical key as equal)
        { Qt::Key_Colon,        0x29 }, // Shift+; (same physical key as semicolon)
        { Qt::Key_Less,         0x2B }, // Shift+, (same physical key as comma)
        { Qt::Key_Greater,      0x2F }, // Shift+. (same physical key as period)
        { Qt::Key_Question,     0x2C }, // Shift+/ (same physical key as slash)
        { Qt::Key_At,           0x13 }, // Shift+2
        { Qt::Key_AsciiCircum,  0x16 }, // Shift+6
        { Qt::Key_Underscore,   0x1B }, // Shift+- (same physical key as minus)
        { Qt::Key_BraceLeft,    0x21 }, // Shift+[ (same physical key as bracket left)
        { Qt::Key_BraceRight,   0x1E }, // Shift+] (same physical key as bracket right)
        { Qt::Key_Bar,          0x2A }, // Shift+\ (same physical key as backslash)
        { Qt::Key_AsciiTilde,   0x32 }, // Shift+` (same physical key as grave)
    };
}
