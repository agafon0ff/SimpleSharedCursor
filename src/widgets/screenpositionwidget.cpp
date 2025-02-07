#include <QGraphicsRectItem>
#include <QDebug>
#include "screenpositionwidget.h"

ScreenPositionWidget::ScreenPositionWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setCacheMode(QGraphicsView::CacheBackground);

    graphicsScene.setSceneRect(graphicsScene.itemsBoundingRect());
    setScene(&graphicsScene);
}

ScreenPositionWidget::~ScreenPositionWidget()
{
}

QVector<ScreenRectItem *> ScreenPositionWidget::screenRectItems() const
{
    return items;
}

void ScreenPositionWidget::addDevice(QSharedPointer<SharedCursor::Device> device)
{
    for (const QRect &rect: std::as_const(device->screens)) {
        ScreenRectItem *item = createRect(rect, device->position);
        item->setText(device->name);
        item->setUuid(device->uuid);
    }
    calculateSceneRect();
}

void ScreenPositionWidget::removeDevice(const QUuid &uuid)
{
    for (ScreenRectItem *item: std::as_const(items)) {
        if (item->uuid() == uuid) {
            disconnect(item, &ScreenRectItem::positionChanged, this, &ScreenPositionWidget::onItemPositionChanged);
            disconnect(item, &ScreenRectItem::released, this, &ScreenPositionWidget::calculateSceneRect);
            items.removeOne(item);
            item->deleteLater();
        }
    }
    calculateSceneRect();
}

void ScreenPositionWidget::clearWidget()
{
    for (ScreenRectItem *item: std::as_const(items)) {
        item->disconnect();
        item->deleteLater();
    }
    items.clear();
}

void ScreenPositionWidget::normalize()
{
     for (ScreenRectItem *item: std::as_const(items)) {
         item->setPos(item->pos() - minPos);
     }
     calculateTransits();
}

void ScreenPositionWidget::onItemPositionChanged(ScreenRectItem *_item, const QPointF &dpos)
{
    for (ScreenRectItem *item: std::as_const(items)) {
        if (_item == item) continue;
        if (item->uuid() == _item->uuid()) {
            item->setPos(item->pos() + dpos);
        }
    }
}

void ScreenPositionWidget::calculateSceneRect()
{
    if (items.isEmpty())
        return;

    ScreenRectItem *item = items.at(0);
    qreal x = item->x(), y = item->y();
    qreal w = item->x() + item->rect().right();
    qreal h = item->y() + item->rect().bottom();
    qreal indent = item->rect().height();

    for (int i=1; i<items.size(); ++i) {
        item = items.at(i);
        if (item->x() < x) x = item->x();
        if (item->y() < y) y = item->y();
        if (item->rect().right() + item->x() > w) w = item->rect().right() + item->x();
        if (item->rect().bottom() + item->y() > h) h = item->rect().bottom() + item->y();
        if (item->rect().height() > indent) indent = item->rect().height();
    }

    w = w - x;
    h = h - y;
    minPos = {x, y};

    QRectF bound(x - indent, y - indent, w + indent*2 , h + indent*2);

    setSceneRect(bound);
    fitInView(bound, Qt::KeepAspectRatio);
    calculateTransits();
}

void ScreenPositionWidget::calculateTransits()
{
    for (ScreenRectItem *item: std::as_const(items)) {
        item->clearTransits();
    }

    for (ScreenRectItem *from: std::as_const(items)) {
        for (ScreenRectItem *to: std::as_const(items)) {
            if (from == to) continue;
            if (from->uuid() == to->uuid()) continue;
            calculateTransitsTopBottom(from, to);
            calculateTransitsLeftRight(from, to);
        }
    }
}

ScreenRectItem *ScreenPositionWidget::createRect(const QRect &rect, const QPoint &pos)
{
    ScreenRectItem *item = new ScreenRectItem();
    scene()->addItem(item);
    item->setRect(rect);
    item->setPos(pos);

    connect(item, &ScreenRectItem::positionChanged, this, &ScreenPositionWidget::onItemPositionChanged);
    connect(item, &ScreenRectItem::released, this, &ScreenPositionWidget::calculateSceneRect);

    items.append(item);

    return item;
}

void ScreenPositionWidget::calculateTransitsTopBottom(ScreenRectItem *itemFrom, ScreenRectItem *itemTo)
{
    const QRectF &from = itemFrom->adjusted();
    const QRectF &to = itemTo->adjusted();
    qreal distance = from.top() - to.bottom();

    QLineF lineFrom = QLineF(from.topLeft(), from.topRight());
    QLineF lineTo = QLineF(to.bottomLeft(), to.bottomRight());

    qreal limit = (lineFrom.dx() + lineTo.dx()) / 6.;

    if (distance >= 0 && distance < limit
            && lineFrom.x2() > lineTo.x1()
            && lineFrom.x1() < lineTo.x2()) {

        itemFrom->addTransit({QLine(qMax(lineFrom.x1() - itemFrom->x(), lineTo.x1() - itemFrom->x()), itemFrom->rect().y(),
                                    qMin(lineFrom.x2() - itemFrom->x(), lineTo.x2() - itemFrom->x()), itemFrom->rect().y()),
                              QPoint(qMax(lineFrom.x1() - itemTo->x(), lineTo.x1() - itemTo->x()), itemTo->rect().bottom()-4),
                              itemTo->uuid()});

        itemTo->addTransit({QLine(qMax(lineFrom.x1() - itemTo->x(), lineTo.x1() - itemTo->x()), itemTo->rect().bottom()-1,
                                  qMin(lineFrom.x2() - itemTo->x(), lineTo.x2() - itemTo->x()), itemTo->rect().bottom()-1),
                              QPoint(qMax(lineFrom.x1() - itemFrom->x(), lineTo.x1() - itemFrom->x()), itemFrom->rect().y()+4),
                              itemFrom->uuid()});
    }
}

void ScreenPositionWidget::calculateTransitsLeftRight(ScreenRectItem *itemFrom, ScreenRectItem *itemTo)
{
    const QRectF &from = itemFrom->adjusted();
    const QRectF &to = itemTo->adjusted();
    qreal distance = from.left() - to.right();

    QLineF lineFrom = QLineF(from.topLeft(), from.bottomLeft());
    QLineF lineTo = QLineF(to.topRight(), to.bottomRight());

    qreal limit = (lineFrom.dy() + lineTo.dy()) / 6.;

    if (distance >= 0 && distance < limit
            && lineFrom.y2() > lineTo.y1()
            && lineFrom.y1() < lineTo.y2()) {

        itemFrom->addTransit({QLine(itemFrom->rect().x(), qMax(lineFrom.y1() - itemFrom->y(), lineTo.y1() - itemFrom->y()),
                                    itemFrom->rect().x(), qMin(lineFrom.y2() - itemFrom->y(), lineTo.y2() - itemFrom->y())),
                              QPoint(itemTo->rect().right()-4, qMax(lineFrom.y1() - itemTo->y(), lineTo.y1() - itemTo->y())),
                              itemTo->uuid()});

        itemTo->addTransit({QLine(itemTo->rect().right()-1, qMax(lineFrom.y1() - itemTo->y(), lineTo.y1() - itemTo->y()),
                                  itemTo->rect().right()-1, qMin(lineFrom.y2() - itemTo->y(), lineTo.y2() - itemTo->y())),
                              QPoint(itemFrom->rect().x()+4, qMax(lineFrom.y1() - itemFrom->y(), lineTo.y1() - itemFrom->y())),
                              itemFrom->uuid()});
    }
}

void ScreenPositionWidget::resizeEvent(QResizeEvent *)
{
    calculateSceneRect();
}

