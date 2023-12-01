#include <QCursor>
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
    display = XOpenDisplay(Q_NULLPTR);

//    unsigned int code = XKeysymToKeycode(display, keycode);
    unsigned int code = keycode;

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
