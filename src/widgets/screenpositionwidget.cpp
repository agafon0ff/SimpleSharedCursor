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

    _graphicsScene.setSceneRect(_graphicsScene.itemsBoundingRect());
    setScene(&_graphicsScene);
}

ScreenPositionWidget::~ScreenPositionWidget()
{
}

QVector<ScreenRectItem *> ScreenPositionWidget::screenRectItems() const
{
    return _items;
}

void ScreenPositionWidget::addDevice(QSharedPointer<SharedCursor::Device> device)
{
    for (const SharedCursor::Screen &screen: std::as_const(device->screens)) {
        ScreenRectItem *item = createScreen(screen, device->position);
        item->setText(device->name);
        item->setUuid(device->uuid);
        item->setIndex(_items.size());
        _items.append(item);
    }
    calculateSceneRect();
}

void ScreenPositionWidget::removeDevice(const QUuid &uuid)
{
    for (ScreenRectItem *item: std::as_const(_items)) {
        if (item->uuid() == uuid) {
            disconnect(item, &ScreenRectItem::positionChanged, this, &ScreenPositionWidget::onItemPositionChanged);
            disconnect(item, &ScreenRectItem::released, this, &ScreenPositionWidget::calculateSceneRect);
            disconnect(item, &ScreenRectItem::selected, this, &ScreenPositionWidget::onItemSelected);
            disconnect(item, &ScreenRectItem::enabled, this, &ScreenPositionWidget::calculateTransits);
            _items.removeOne(item);
            item->deleteLater();
        }
    }
    calculateSceneRect();
}

void ScreenPositionWidget::clearWidget()
{
    for (ScreenRectItem *item: std::as_const(_items)) {
        item->disconnect();
        item->deleteLater();
    }
    _items.clear();
}

void ScreenPositionWidget::normalize()
{
     for (ScreenRectItem *item: std::as_const(_items)) {
         item->setPos(item->pos() - _minPos);
     }
     calculateTransits();
}

void ScreenPositionWidget::onItemSelected(ScreenRectItem *item, bool state)
{
    if (!state)
        return;

    for (ScreenRectItem *_item: std::as_const(_items))
        _item->setZValue(item->uuid() == _item->uuid());
}

void ScreenPositionWidget::onItemPositionChanged(ScreenRectItem *_item, const QPointF &dpos)
{
    for (ScreenRectItem *item: std::as_const(_items)) {
        if (_item == item) continue;
        if (item->uuid() == _item->uuid()) {
            item->setPos(item->pos() + dpos);
        }
    }
}

void ScreenPositionWidget::calculateSceneRect()
{
    if (_items.isEmpty())
        return;

    ScreenRectItem *item = _items.at(0);
    qreal x = item->x() + item->rect().x();
    qreal y = item->y() + item->rect().y();
    qreal w = item->x() + item->rect().right();
    qreal h = item->y() + item->rect().bottom();
    qreal indent = item->rect().height();

    for (int i=1; i<_items.size(); ++i) {
        item = _items.at(i);

        qreal itX = item->x() + item->rect().x();
        qreal itY = item->y() + item->rect().y();
        qreal itW = item->x() + item->rect().right();
        qreal itH = item->y() + item->rect().bottom();

        if (itX < x) x = itX;
        if (itY < y) y = itY;
        if (itW > w) w = itW;
        if (itH > h) h = itH;

        if (item->rect().height() > indent) indent = item->rect().height();
    }

    w = w - x;
    h = h - y;
    _minPos = {x, y};

    QRectF bound(x - indent/2, y - indent/2, w + indent, h + indent);

    setSceneRect(bound);
    centerOn(bound.center());

    fitInView(bound, Qt::KeepAspectRatio);
    calculateTransits();
}

void ScreenPositionWidget::calculateTransits()
{
    for (ScreenRectItem *item: std::as_const(_items)) {
        item->clearTransits();
    }

    for (ScreenRectItem *from: std::as_const(_items)) {
        for (ScreenRectItem *to: std::as_const(_items)) {
            if (from == to) continue;
            if (from->uuid() == to->uuid()) continue;
            if (!from->isEnabled() || !to->isEnabled()) continue;
            calculateTransitsTopBottom(from, to);
            calculateTransitsLeftRight(from, to);
        }
    }
}

ScreenRectItem *ScreenPositionWidget::createScreen(const SharedCursor::Screen &screen, const QPoint &pos)
{
    ScreenRectItem *item = new ScreenRectItem();
    scene()->addItem(item);
    item->setRect(screen.rect);
    item->setEnabled(screen.enabled);
    item->setPos(pos);

    connect(item, &ScreenRectItem::positionChanged, this, &ScreenPositionWidget::onItemPositionChanged);
    connect(item, &ScreenRectItem::selected, this, &ScreenPositionWidget::onItemSelected);
    connect(item, &ScreenRectItem::released, this, &ScreenPositionWidget::calculateSceneRect);
    connect(item, &ScreenRectItem::enabled, this, &ScreenPositionWidget::calculateTransits);

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

