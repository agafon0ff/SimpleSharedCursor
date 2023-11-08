#include "screenrectitem.h"
#include <QDebug>

ScreenRectItem::ScreenRectItem(QGraphicsItem *parent)
    : QGraphicsObject{parent}
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
}

void ScreenRectItem::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}

QUuid ScreenRectItem::uuid() const
{
    return _uuid;
}

QPoint ScreenRectItem::position()
{
    return QPoint(x(), y());
}

void ScreenRectItem::setRects(const QVector<QRect> &rects)
{
    _rects = rects;

    int x = 0, y = 0, w = 1, h = 1;
    for (const QRect &rect: qAsConst(rects)) {
        if (rect.x() < x) x = rect.x();
        if (rect.y() < y) y = rect.y();
        if (rect.width() + rect.x() > w) w = rect.width() + rect.x();
        if (rect.height() + rect.y() > h) h = rect.height() + rect.y();
    }

    _size = QSizeF(w, h);
    update();
}

QVector<QRect> ScreenRectItem::rects() const
{
    return _rects;
}

void ScreenRectItem::setGeometry(const QRect &rect)
{
    setPos(rect.topLeft());
    _size = rect.size();
    prepareGeometryChange();
    update();
}

QRect ScreenRectItem::geometry() const
{
    return QRect(x(), y(), _size.width(), _size.height());
}

int ScreenRectItem::width() const
{
    return _size.width();
}

int ScreenRectItem::height() const
{
    return _size.height();
}

void ScreenRectItem::setBackgroundColor(const QColor &color)
{
    backgroundColor = color;
    update();
}

void ScreenRectItem::setText(const QString &text)
{
    _text = text;
    update();
}

bool intersectRight(const QRect &from, const QRect &to, QLine &result)
{
    int adjust = from.width() / 10;
    QRect rect(from.right() - adjust, from.y(), adjust * 2, from.height());
    QRect intersect = rect.intersected(to);
    result.setLine(from.right(), intersect.top(), from.right(), intersect.bottom());
    return intersect.height() > 0;
}

bool intersectLeft(const QRect &from, const QRect &to, QLine &result)
{
    int adjust = from.width() / 10;
    QRect rect(from.left() - adjust, from.y(), adjust * 2, from.height());
    QRect intersect = rect.intersected(to);
    result.setLine(from.left(), intersect.top(), from.left(), intersect.bottom());
    return intersect.height() > 0;
}

bool intersectTop(const QRect &from, const QRect &to, QLine &result)
{
    int adjust = from.height() / 10;
    QRect rect(from.x(), from.top() - adjust, from.width(), adjust * 2);
    QRect intersect = rect.intersected(to);
    result.setLine(intersect.left(), from.top(), intersect.right(), from.top());
    return intersect.width() > 0;
}

bool intersectBottom(const QRect &from, const QRect &to, QLine &result)
{
    int adjust = from.height() / 10;
    QRect rect(from.x(), from.bottom() - adjust, from.width(), adjust * 2);
    QRect intersect = rect.intersected(to);
    result.setLine(intersect.left(), from.bottom(), intersect.right(), from.bottom());
    return intersect.width() > 0;
}

void ScreenRectItem::calculateTransits(const QUuid &uuid, const QRect &rect)
{
    QLine line;
    QRect adjusted = rect.adjusted(-x(), -y(), -x(), -y());

    for (const QRect &r: qAsConst(_rects)) {
        if (intersectRight(r, adjusted, line)) _transits.append({line, uuid});
        if (intersectLeft(r, adjusted, line)) _transits.append({line, uuid});
        if (intersectTop(r, adjusted, line)) _transits.append({line, uuid});
        if (intersectBottom(r, adjusted, line)) _transits.append({line, uuid});
    }
}

void ScreenRectItem::clearTransits()
{
    _transits.clear();
    update();
}

QVector<Transit> ScreenRectItem::transits() const
{
    return _transits;
}

QRectF ScreenRectItem::boundingRect() const
{
    return QRectF(0, 0, _size.width(), _size.height());
}

QPainterPath ScreenRectItem::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, _size.width(), _size.height());
    return path;
}

void ScreenRectItem::paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(item);

    qreal penWidth = 2 / p->transform().m11();

    p->setPen(QPen(QBrush(QColor(0, 0, 0)), penWidth));
    p->setBrush(backgroundColor);

    for (const QRect &rect: qAsConst(_rects)) {
        p->drawRect(rect.x() + penWidth/2,
                    rect.y() + penWidth/2,
                    rect.width() - penWidth,
                    rect.height() - penWidth);
    }

    QFont font = p->font();
    font.setPixelSize(200);
    p->setFont(font);
    p->drawText(QRect(0, 0, _size.width(), _size.height()), Qt::AlignCenter, _text);

    p->setPen(QPen(QBrush(QColor(255, 0, 0)), penWidth));

    for (const auto &transit: qAsConst(_transits)) {
        p->drawLine(transit.line);
    }
}

QVariant ScreenRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && !isPositionChanged) {
        isPositionChanged = true;
        emit movementHasStarted(this);
    }
    else if (change == ItemSelectedChange) {
        setZValue(isSelected() ? 1 : 2);
    }

    return QGraphicsItem::itemChange(change, value);
}

void ScreenRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (isPositionChanged) {
        isPositionChanged = false;
        emit movementHasFinished(this);
    }

    QGraphicsObject::mouseReleaseEvent(event);
}
