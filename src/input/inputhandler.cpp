#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QCursor>
#include <QDebug>

#include "inputhandler.h"
#include "global.h"

InputHandler::InputHandler(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::BlankCursor);
    setMouseTracking(true);

    _jsonKey[SharedCursor::KEY_TYPE] = SharedCursor::KEY_INPUT;
}

void InputHandler::setUuid(const QUuid &uuid)
{
    _ownUuid = uuid;
}

void InputHandler::setCenterIn(const QPoint &pos)
{
    move(pos.x() - width() / 2, pos.y() - height() / 2);
}

void InputHandler::setRemoteControlState(const QUuid &master, const QUuid &slave)
{
    _remoteUuid = slave;
    _isActive = master != slave && _ownUuid == master;

    if (_isActive) {
        show();
        setFocus();
        activateWindow();
    }
    else {
        hide();
    }
}

void InputHandler::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush({0, 0, 0, 1}));
    p.drawRect(0, 0, width(), height());
    p.end();
}

bool InputHandler::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::Hide:
    case QEvent::WindowDeactivate:
        if (_isActive) {
            show();
            setFocus();
            activateWindow();
        }
        break;
    case QEvent::Close:
        sendKeyEventMessage(Qt::Key_F4, true);
        sendKeyEventMessage(Qt::Key_F4, false);
        event->ignore();
        break;
    case QEvent::KeyPress:
        keyStateChanged(dynamic_cast<QKeyEvent*>(event), true);
        break;
    case QEvent::KeyRelease:
        keyStateChanged(dynamic_cast<QKeyEvent*>(event), false);
        break;
    case QEvent::MouseButtonPress:
        mouseStateChanged(dynamic_cast<QMouseEvent*>(event), true);
        break;
    case QEvent::MouseButtonRelease:
        mouseStateChanged(dynamic_cast<QMouseEvent*>(event), false);
        break;
    case QEvent::MouseButtonDblClick:
        mouseStateChanged(dynamic_cast<QMouseEvent*>(event), true);
        mouseStateChanged(dynamic_cast<QMouseEvent*>(event), false);
        break;
    case QEvent::Wheel:
        wheelStateChanged(dynamic_cast<QWheelEvent*>(event));
        break;
    default:
//        qDebug() << Q_FUNC_INFO << event->type();
        return QWidget::event(event);
    }

    return false;
}

void InputHandler::sendKeyEventMessage(int keycode, bool pressed)
{
    _jsonKey[SharedCursor::KEY_INPUT] = SharedCursor::KEY_KEYBOARD;
    _jsonKey[SharedCursor::KEY_VALUE] = keycode;
    _jsonKey[SharedCursor::KEY_PRESSED] = pressed;
    emit message(_remoteUuid, _jsonKey);
}

void InputHandler::keyStateChanged(QKeyEvent *event, bool pressed)
{
    sendKeyEventMessage(static_cast<int>(event->key()), pressed);
}

void InputHandler::mouseStateChanged(QMouseEvent *event, bool pressed)
{
    _jsonKey[SharedCursor::KEY_INPUT] = SharedCursor::KEY_MOUSE;
    _jsonKey[SharedCursor::KEY_PRESSED] = pressed;

    switch(event->button()) {
    case Qt::LeftButton: _jsonKey[SharedCursor::KEY_VALUE] = 0; break;
    case Qt::MiddleButton: _jsonKey[SharedCursor::KEY_VALUE] = 1; break;
    case Qt::RightButton: _jsonKey[SharedCursor::KEY_VALUE] = 2; break;
    default: break;
    }

    emit message(_remoteUuid, _jsonKey);
}

void InputHandler::wheelStateChanged(QWheelEvent *event)
{
    _jsonKey[SharedCursor::KEY_INPUT] = SharedCursor::KEY_WHEEL;
    _jsonKey[SharedCursor::KEY_VALUE] = static_cast<int>(event->angleDelta().y());
    emit message(_remoteUuid, _jsonKey);
}
