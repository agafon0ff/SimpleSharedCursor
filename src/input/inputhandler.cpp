#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QCursor>
#include <QDebug>

#include "inputhandler.h"

InputHandler::InputHandler(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::BlankCursor);
    setMouseTracking(true);

    jsonKey[SharedCursor::KEY_TYPE] = SharedCursor::KEY_INPUT;
}

void InputHandler::holdCursor(const QUuid &uuid, bool state)
{
    remoteUuid = uuid;
    isActive = state;

    if (state) {
        show();
        setFocus();
        activateWindow();
    }
    else {
        hide();
    }
}

void InputHandler::setCenterIn(const QPoint &pos)
{
    move(pos.x() - width() / 2, pos.y() - height() / 2);
}

bool InputHandler::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::Hide:
    case QEvent::WindowDeactivate:
        if (isActive) {
            show();
            setFocus();
            activateWindow();
        }
        break;
    case QEvent::Close:
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

void InputHandler::keyStateChanged(QKeyEvent *event, bool pressed)
{
    jsonKey[SharedCursor::KEY_INPUT] = SharedCursor::KEY_KEYBOARD;
    jsonKey[SharedCursor::KEY_VALUE] = static_cast<int>(event->nativeScanCode());
    jsonKey[SharedCursor::KEY_PRESSED] = pressed;
    emit message(remoteUuid, jsonKey);
}

void InputHandler::mouseStateChanged(QMouseEvent *event, bool pressed)
{
    jsonKey[SharedCursor::KEY_INPUT] = SharedCursor::KEY_MOUSE;
    jsonKey[SharedCursor::KEY_PRESSED] = pressed;

    switch(event->button()) {
    case Qt::LeftButton: jsonKey[SharedCursor::KEY_VALUE] = 0; break;
    case Qt::MiddleButton: jsonKey[SharedCursor::KEY_VALUE] = 1; break;
    case Qt::RightButton: jsonKey[SharedCursor::KEY_VALUE] = 2; break;
    default: break;
    }

    emit message(remoteUuid, jsonKey);
}

void InputHandler::wheelStateChanged(QWheelEvent *event)
{
    jsonKey[SharedCursor::KEY_INPUT] = SharedCursor::KEY_WHEEL;
    jsonKey[SharedCursor::KEY_VALUE] = static_cast<int>(event->angleDelta().y());
    emit message(remoteUuid, jsonKey);
}
