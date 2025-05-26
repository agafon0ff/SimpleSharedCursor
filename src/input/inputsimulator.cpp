#include <QCursor>
#include <QDebug>
#include "inputsimulator.h"

#ifdef Q_OS_WIN
#include "windows.h"
#include "winuser.h"
#endif

#ifdef Q_OS_UNIX
//sudo apt install libxtst-dev
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

InputSimulator::InputSimulator(QObject *parent)
    : QObject{parent}
{
    createKeymap();
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

#ifdef Q_OS_UNIX
    Display *display = XOpenDisplay(nullptr);

    unsigned char code = 0;

    auto it = _keymap.find(keycode);
    if (it != _keymap.end()) {
        code = XKeysymToKeycode(display, it.value());
    }
    else {
        code = static_cast<unsigned char>(keycode);
    }

    XTestFakeKeyEvent(display, code, state, 0);
    XFlush(display);
    XCloseDisplay(display);
#endif

#ifdef Q_OS_WIN
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    auto it = keymap.find(keycode);
    if (it != keymap.end()) {
        ip.ki.wVk = it.value();
    }
    else {
        ip.ki.wVk = static_cast<unsigned short>(keycode);
    }

    if (state) {
        ip.ki.dwFlags = 0;
    }
    else {
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
    }

    SendInput(1, &ip, sizeof(INPUT));
#endif
}

void InputSimulator::setMouseEvent(int button, bool state)
{
    if (!_releaseProcess) {
        if (state) _pressedMouse.append(button);
        else _pressedMouse.removeOne(button);
    }

#ifdef Q_OS_UNIX
    Display *display = XOpenDisplay(nullptr);

    if (button == 0) { //left
        XTestFakeButtonEvent(display, Button1, state, 0);
    }
    else if(button == 1) { //middle
        XTestFakeButtonEvent(display, Button2, state, 0);
    }
    else if(button == 2) { //right
        XTestFakeButtonEvent(display, Button3, state, 0);
    }

    XFlush(display);
    XCloseDisplay(display);
#endif

#ifdef Q_OS_WIN
    INPUT ip;

    ZeroMemory(&ip,sizeof(ip));

    ip.type = INPUT_MOUSE;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    if (button == 0) { //left
        if(state)ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        else ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    }
    else if(button == 1) { //middle
        if(state)ip.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
        else ip.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
    }
    else if(button == 2) { //right
        if(state)ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        else ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    }

    SendInput(1, &ip, sizeof(INPUT));
#endif
}

void InputSimulator::setWheelEvent(int delta)
{
#ifdef Q_OS_UNIX
    Display *display = XOpenDisplay(nullptr);
    quint32 btnNum = delta < 0 ? Button5 : Button4;

    XTestFakeButtonEvent(display, btnNum, true, 0);
    XTestFakeButtonEvent(display, btnNum, false, 0);

    XFlush(display);
    XCloseDisplay(display);
#endif

#ifdef Q_OS_WIN
    INPUT ip;

    ZeroMemory(&ip,sizeof(ip));

    ip.type = INPUT_MOUSE;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    ip.mi.dwFlags = MOUSEEVENTF_WHEEL;
    ip.mi.mouseData = static_cast<DWORD>(delta);

    SendInput(1, &ip, sizeof(INPUT));
#endif
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
#ifdef Q_OS_UNIX
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
#endif

#ifdef Q_OS_WIN
    keymap = {
        { Qt::Key_Escape, VK_ESCAPE },
        { Qt::Key_Tab, VK_TAB },
        { Qt::Key_Backspace, VK_BACK },
        { Qt::Key_Return, VK_RETURN },
        { Qt::Key_Enter, VK_RETURN },
        { Qt::Key_Insert, VK_INSERT },
        { Qt::Key_Delete, VK_DELETE },
        { Qt::Key_Pause, VK_PAUSE },
        { Qt::Key_Print, VK_PRINT },
        { Qt::Key_Home, VK_HOME },
        { Qt::Key_End, VK_END },
        { Qt::Key_Left, VK_LEFT },
        { Qt::Key_Up, VK_UP },
        { Qt::Key_Right, VK_RIGHT },
        { Qt::Key_Down, VK_DOWN },
        { Qt::Key_PageUp, VK_PRIOR },
        { Qt::Key_PageDown, VK_NEXT },
        { Qt::Key_Shift, VK_SHIFT },
        { Qt::Key_Control, VK_CONTROL },
        { Qt::Key_Alt, VK_MENU },
        { Qt::Key_CapsLock, VK_CAPITAL },
        { Qt::Key_NumLock, VK_NUMLOCK },
        { Qt::Key_ScrollLock, VK_SCROLL },
        { Qt::Key_F1, VK_F1 },
        { Qt::Key_F2, VK_F2 },
        { Qt::Key_F3, VK_F3 },
        { Qt::Key_F4, VK_F4 },
        { Qt::Key_F5, VK_F5 },
        { Qt::Key_F6, VK_F6 },
        { Qt::Key_F7, VK_F7 },
        { Qt::Key_F8, VK_F8 },
        { Qt::Key_F9, VK_F9 },
        { Qt::Key_F10, VK_F10 },
        { Qt::Key_F11, VK_F11 },
        { Qt::Key_F12, VK_F12 },
        { Qt::Key_F13, VK_F13 },
        { Qt::Key_F14, VK_F14 },
        { Qt::Key_Menu, VK_MENU },
        { Qt::Key_Help, VK_HELP },
        { Qt::Key_Space, VK_SPACE },
        { Qt::Key_Exclam, 0x31 },
        { Qt::Key_QuoteDbl, VK_OEM_7 },
        { Qt::Key_NumberSign, 0x33 },
        { Qt::Key_Dollar, 0x34 },
        { Qt::Key_Percent, 0x35 },
        { Qt::Key_Ampersand, 0x37 },
        { Qt::Key_Apostrophe, VK_OEM_7 },
        { Qt::Key_ParenLeft, 0x39 },
        { Qt::Key_ParenRight, 0x30 },
        { Qt::Key_Asterisk, 0x38 },
        { Qt::Key_Plus, VK_OEM_PLUS },
        { Qt::Key_Comma, VK_OEM_COMMA },
        { Qt::Key_Minus, VK_OEM_MINUS },
        { Qt::Key_Period, VK_OEM_PERIOD },
        { Qt::Key_Slash, VK_OEM_2 },
        { Qt::Key_0, 0x30 },
        { Qt::Key_1, 0x31 },
        { Qt::Key_2, 0x32 },
        { Qt::Key_3, 0x33 },
        { Qt::Key_4, 0x34 },
        { Qt::Key_5, 0x35 },
        { Qt::Key_6, 0x36 },
        { Qt::Key_7, 0x37 },
        { Qt::Key_8, 0x38 },
        { Qt::Key_9, 0x39 },
        { Qt::Key_Colon, VK_OEM_1 },
        { Qt::Key_Semicolon, VK_OEM_1 },
        { Qt::Key_Less, VK_OEM_COMMA },
        { Qt::Key_Equal, VK_OEM_PLUS },
        { Qt::Key_Greater, VK_OEM_PERIOD },
        { Qt::Key_Question, VK_OEM_2 },
        { Qt::Key_At, 0x32 },
        { Qt::Key_A, 0x41 },
        { Qt::Key_B, 0x42 },
        { Qt::Key_C, 0x43 },
        { Qt::Key_D, 0x44 },
        { Qt::Key_E, 0x45 },
        { Qt::Key_F, 0x46 },
        { Qt::Key_G, 0x47 },
        { Qt::Key_H, 0x48 },
        { Qt::Key_I, 0x49 },
        { Qt::Key_J, 0x4a },
        { Qt::Key_K, 0x4b },
        { Qt::Key_L, 0x4c },
        { Qt::Key_M, 0x4d },
        { Qt::Key_N, 0x4e },
        { Qt::Key_O, 0x4f },
        { Qt::Key_P, 0x50 },
        { Qt::Key_Q, 0x51 },
        { Qt::Key_R, 0x52 },
        { Qt::Key_S, 0x53 },
        { Qt::Key_T, 0x54 },
        { Qt::Key_U, 0x55 },
        { Qt::Key_V, 0x56 },
        { Qt::Key_W, 0x57 },
        { Qt::Key_X, 0x58 },
        { Qt::Key_Y, 0x59 },
        { Qt::Key_Z, 0x5a },
        { Qt::Key_BracketLeft, VK_OEM_4 },
        { Qt::Key_Backslash, VK_OEM_5 },
        { Qt::Key_BracketRight, VK_OEM_6 },
        { Qt::Key_AsciiCircum, 0x36 },
        { Qt::Key_Underscore, VK_OEM_MINUS },
        { Qt::Key_QuoteLeft, VK_OEM_3 },
        { Qt::Key_BraceLeft, VK_OEM_4 },
        { Qt::Key_Bar, VK_OEM_5 },
        { Qt::Key_BraceRight, VK_OEM_6 },
        { Qt::Key_AsciiTilde, VK_OEM_3 }
    };
#endif
}
