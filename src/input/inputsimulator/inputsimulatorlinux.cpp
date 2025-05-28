#include <QtGlobal>
#if defined(Q_OS_LINUX)

#include <QCursor>
#include <QDebug>
#include "inputsimulator.h"

//sudo apt install libxtst-dev
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

InputSimulator::InputSimulator(QObject *parent)
    : QObject{parent}
{
    _display = XOpenDisplay(nullptr);
    createKeymap();
}

InputSimulator::~InputSimulator()
{
    if (_display)
        XCloseDisplay(_display);
}

void InputSimulator::setControlState(SharedCursor::ControlState state)
{
    if (_controlState != state) {
        if (_controlState == SharedCursor::Slave) {
            releasePressedKeys();
        }
        _controlState = state;
    }
}

void InputSimulator::setCutsorPosition(const QPoint &pos)
{
    QCursor::setPos(pos);
}

void InputSimulator::setCutsorDelta(const QPoint &pos)
{
    QCursor::setPos(QCursor::pos() + pos);
}

void InputSimulator::setKeyboardEvent(int keycode, bool state)
{
    if (!_releaseProcess) {
        if (state) _pressedKeys.append(keycode);
        else _pressedKeys.removeOne(keycode);
    }

    if (!_display)
        return;

    unsigned char code = 0;

    auto it = _keymap.find(keycode);
    if (it != _keymap.end()) {
        code = XKeysymToKeycode(_display, it.value());
    }
    else {
        code = static_cast<unsigned char>(keycode);
    }

    XTestFakeKeyEvent(_display, code, state, 0);
    XFlush(_display);
}

void InputSimulator::setMouseEvent(int button, bool state)
{
    if (!_releaseProcess) {
        if (state) _pressedMouse.append(button);
        else _pressedMouse.removeOne(button);
    }

    if (!_display)
        return;

    if (button == 0) { //left
        XTestFakeButtonEvent(_display, Button1, state, 0);
    }
    else if(button == 1) { //middle
        XTestFakeButtonEvent(_display, Button2, state, 0);
    }
    else if(button == 2) { //right
        XTestFakeButtonEvent(_display, Button3, state, 0);
    }

    XFlush(_display);
}

void InputSimulator::setWheelEvent(int delta)
{
    Display *display = XOpenDisplay(nullptr);
    quint32 btnNum = delta < 0 ? Button5 : Button4;

    XTestFakeButtonEvent(display, btnNum, true, 0);
    XTestFakeButtonEvent(display, btnNum, false, 0);

    XFlush(display);
    XCloseDisplay(display);
}

void InputSimulator::releasePressedKeys()
{
    _releaseProcess = true;

    if (!_pressedKeys.empty()) {
        for (int code: std::as_const(_pressedKeys)) {
            setKeyboardEvent(code, false);
        }
    }

    if (!_pressedMouse.empty()) {
        for (int button: std::as_const(_pressedMouse)) {
            setMouseEvent(button, false);
        }
    }

    _releaseProcess = false;
}

void InputSimulator::createKeymap()
{
    _keymap = {
        { Qt::Key_Escape, XK_Escape },
        { Qt::Key_Tab, XK_Tab },
        { Qt::Key_Backspace, XK_BackSpace },
        { Qt::Key_Return, XK_Return },
        { Qt::Key_Enter, XK_Return },
        { Qt::Key_Insert, XK_Insert },
        { Qt::Key_Delete, XK_Delete },
        { Qt::Key_Pause, XK_Pause },
        { Qt::Key_Print, XK_Print },
        { Qt::Key_Home, XK_Home },
        { Qt::Key_End, XK_End },
        { Qt::Key_Left, XK_Left },
        { Qt::Key_Up, XK_Up },
        { Qt::Key_Right, XK_Right },
        { Qt::Key_Down, XK_Down },
        { Qt::Key_PageUp, XK_Page_Up },
        { Qt::Key_PageDown, XK_Page_Down },
        { Qt::Key_Shift, XK_Shift_L },
        { Qt::Key_Control, XK_Control_L },
        { Qt::Key_Meta, XK_Meta_L },
        { Qt::Key_Alt, XK_Alt_L },
        { Qt::Key_CapsLock, XK_Caps_Lock },
        { Qt::Key_NumLock, XK_Num_Lock },
        { Qt::Key_ScrollLock, XK_Scroll_Lock },
        { Qt::Key_F1, XK_F1 },
        { Qt::Key_F2, XK_F2 },
        { Qt::Key_F3, XK_F3 },
        { Qt::Key_F4, XK_F4 },
        { Qt::Key_F5, XK_F5 },
        { Qt::Key_F6, XK_F6 },
        { Qt::Key_F7, XK_F7 },
        { Qt::Key_F8, XK_F8 },
        { Qt::Key_F9, XK_F9 },
        { Qt::Key_F10, XK_F10 },
        { Qt::Key_F11, XK_F11 },
        { Qt::Key_F12, XK_F12 },
        { Qt::Key_F13, XK_F13 },
        { Qt::Key_F14, XK_F14 },
        { Qt::Key_Super_L, XK_Super_L },
        { Qt::Key_Super_R, XK_Super_R },
        { Qt::Key_Menu, XK_Menu },
        { Qt::Key_Hyper_L, XK_Hyper_L },
        { Qt::Key_Hyper_R, XK_Hyper_R },
        { Qt::Key_Help, XK_Help },
        { Qt::Key_Space, XK_space },
        { Qt::Key_Exclam, XK_exclam },
        { Qt::Key_QuoteDbl, XK_quotedbl },
        { Qt::Key_NumberSign, XK_numbersign },
        { Qt::Key_Dollar, XK_dollar },
        { Qt::Key_Percent, XK_percent },
        { Qt::Key_Ampersand, XK_ampersand },
        { Qt::Key_Apostrophe, XK_apostrophe },
        { Qt::Key_ParenLeft, XK_parenleft },
        { Qt::Key_ParenRight, XK_parenright },
        { Qt::Key_Asterisk, XK_asterisk },
        { Qt::Key_Plus, XK_plus },
        { Qt::Key_Comma, XK_comma },
        { Qt::Key_Minus, XK_minus },
        { Qt::Key_Period, XK_period },
        { Qt::Key_Slash, XK_slash },
        { Qt::Key_0, XK_0 },
        { Qt::Key_1, XK_1 },
        { Qt::Key_2, XK_2 },
        { Qt::Key_3, XK_3 },
        { Qt::Key_4, XK_4 },
        { Qt::Key_5, XK_5 },
        { Qt::Key_6, XK_6 },
        { Qt::Key_7, XK_7 },
        { Qt::Key_8, XK_8 },
        { Qt::Key_9, XK_9 },
        { Qt::Key_Colon, XK_colon },
        { Qt::Key_Semicolon, XK_semicolon },
        { Qt::Key_Less, XK_comma },
        { Qt::Key_Equal, XK_equal },
        { Qt::Key_Greater, XK_greater },
        { Qt::Key_Question, XK_question },
        { Qt::Key_At, XK_at },
        { Qt::Key_A, XK_A },
        { Qt::Key_B, XK_B },
        { Qt::Key_C, XK_C },
        { Qt::Key_D, XK_D },
        { Qt::Key_E, XK_E },
        { Qt::Key_F, XK_F },
        { Qt::Key_G, XK_G },
        { Qt::Key_H, XK_H },
        { Qt::Key_I, XK_I },
        { Qt::Key_J, XK_J },
        { Qt::Key_K, XK_K },
        { Qt::Key_L, XK_L },
        { Qt::Key_M, XK_M },
        { Qt::Key_N, XK_N },
        { Qt::Key_O, XK_O },
        { Qt::Key_P, XK_P },
        { Qt::Key_Q, XK_Q },
        { Qt::Key_R, XK_R },
        { Qt::Key_S, XK_S },
        { Qt::Key_T, XK_T },
        { Qt::Key_U, XK_U },
        { Qt::Key_V, XK_V },
        { Qt::Key_W, XK_W },
        { Qt::Key_X, XK_X },
        { Qt::Key_Y, XK_Y },
        { Qt::Key_Z, XK_Z },
        { Qt::Key_BracketLeft, XK_bracketleft },
        { Qt::Key_Backslash, XK_backslash },
        { Qt::Key_BracketRight, XK_bracketright },
        { Qt::Key_AsciiCircum, XK_asciicircum },
        { Qt::Key_Underscore, XK_underscore },
        { Qt::Key_QuoteLeft, XK_quoteleft },
        { Qt::Key_BraceLeft, XK_braceleft },
        { Qt::Key_Bar, XK_bar },
        { Qt::Key_BraceRight, XK_braceright },
        { Qt::Key_AsciiTilde, XK_asciitilde }
    };
}
#endif
