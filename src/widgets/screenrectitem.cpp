#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QDebug>
#include <QMenu>

#include <qmath.h>

#include "screenrectitem.h"

ScreenRectItem::ScreenRectItem(QGraphicsItem *parent)
    : QObject(nullptr)
    , QGraphicsRectItem{parent}
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
}

QUuid ScreenRectItem::uuid() const
{
    return _uuid;
}

void ScreenRectItem::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}

QString ScreenRectItem::text() const
{
    return _text;
}

void ScreenRectItem::setText(const QString &text)
{
    _text = text;
    update();
}

int ScreenRectItem::index()
{
    return _index;
}

void ScreenRectItem::setIndex(int index)
{
    _index = index;
}

bool ScreenRectItem::isEnabled()
{
    return _enabled;
}

void ScreenRectItem::setEnabled(bool state)
{
    _enabled = state;
}

void ScreenRectItem::addTransit(SharedCursor::Transit transit)
{
    _transits.append(transit);
    update();
}

void ScreenRectItem::clearTransits()
{
    _transits.clear();
    update();
}

QVector<SharedCursor::Transit> ScreenRectItem::transits() const
{
    return _transits;
}

qreal ScreenRectItem::width() const
{
    return rect().width();
}

qreal ScreenRectItem::height() const
{
    return rect().height();
}

QRectF ScreenRectItem::adjusted()
{
    return rect().adjusted(x(), y(), x(), y());
}

void ScreenRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit pressed(this);
    QGraphicsRectItem::mousePressEvent(event);
}

void ScreenRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit released(this);
    QGraphicsRectItem::mouseReleaseEvent(event);
}

void ScreenRectItem::paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(item);
    Q_UNUSED(p);

    bool isSelected = item->state & QStyle::State_Selected;
    int opacity = _enabled ? 255 : 30;

    qreal penWidth = 2 / p->transform().m11();
    p->setPen(QPen(QBrush(QColor(0, 0, 0, opacity)), penWidth));
    p->setBrush(QBrush(QColor(160, 160, 160, opacity)));

    p->drawRect(rect());

    _font = p->font();
    _font.setPixelSize(penWidth * 7);

    QFontMetrics fm(_font);
    QString eText = fm.elidedText(_text, Qt::ElideRight, static_cast<int>(rect().width()));

    p->setFont(_font);
    p->drawText(rect().adjusted(penWidth + 2, 2, 0, 0), Qt::AlignLeft | Qt::AlignTop, eText);

    p->setPen(QPen(QBrush(QColor(255, 0, 0)), penWidth));

    for (const auto &transit: std::as_const(_transits)) {
        p->drawLine(transit.line);
    }

    if (_selected != isSelected) {
        _selected = isSelected;
        emit selected(this, _selected);
    }
}

QVariant ScreenRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && _selected) {
        emit positionChanged(this, value.toPointF() - pos());
    }

    return QGraphicsItem::itemChange(change, value);
}

void ScreenRectItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    QMenu menu;
    QString text = _enabled ? "disable" : "enable";

    menu.addAction(text, this, [this](){
        _enabled = !_enabled;
        update();
        emit enabled(this, _enabled);
    });

    menu.exec(QCursor::pos());
}

