#include <QCursor>
#include <QDebug>
#include "inputsimulator.h"

#ifdef Q_OS_WIN
#include "windows.h"
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
#ifdef Q_OS_UNIX

    Display *display;
    display = XOpenDisplay(nullptr);
    unsigned char code = 0;

    auto it = keymap.find(keycode);
    if (it != keymap.end()) {
        code = XKeysymToKeycode(display, it.value());
    }
    else {
        code = static_cast<unsigned char>(keycode);
    }

    if (state) {
        XTestFakeKeyEvent(display, code, True, 0);
    }
    else {
        XTestFakeKeyEvent(display, code, False, 0);
    }

    XFlush(display);
    XCloseDisplay(display);
#endif

    XK_BackSpace;

#ifdef Q_OS_WIN
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    ip.ki.wVk = static_cast<unsigned short>(keycode);

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
#ifdef Q_OS_UNIX
    Display *display;
    display = XOpenDisplay(Q_NULLPTR);

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
    Display *display;
    display = XOpenDisplay(Q_NULLPTR);

    quint32 btnNum = Button4;
    if (delta < 0) {
        btnNum = Button5;
    }

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

void InputSimulator::createKeymap()
{
#ifdef Q_OS_UNIX
    keymap = {
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
}
