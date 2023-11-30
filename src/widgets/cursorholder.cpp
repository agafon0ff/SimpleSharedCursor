#include <QPainter>
#include <QCursor>
#include <QDebug>

#include "cursorholder.h"

CursorHolder::CursorHolder(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::BlankCursor);
}

void CursorHolder::holdCursor(bool state)
{
    if (state) {
        show();
        activateWindow();

    }
    else {
        hide();
    }
}

void CursorHolder::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrush(QColor(100, 100, 100, 100));
    p.drawRect(-5, -5, width() + 10, height() + 10);
    p.end();
}
