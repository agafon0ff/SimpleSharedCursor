#include "screenpositionwidget.h"

ScreenPositionWidget::ScreenPositionWidget(QWidget *parent)
    : QGraphicsView{parent}
{
    setRenderHint(QPainter::Antialiasing);
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
    clearWidget();
}

QVector<ScreenRectItem *> ScreenPositionWidget::screenRectItems() const
{
    return items;
}

void ScreenPositionWidget::addDevice(QSharedPointer<Device> device)
{
    ScreenRectItem *item = new ScreenRectItem;
    item->setUuid(device->uuid);
    item->setRects(device->screens);
    item->setText(device->name);
    item->setPos(device->position.x(), device->position.y());
    scene()->addItem(item);
    items.append(item);

    connect(item, &ScreenRectItem::movementHasStarted, this, &ScreenPositionWidget::clearIntersections);
    connect(item, &ScreenRectItem::movementHasFinished, this, &ScreenPositionWidget::onScreenPositionChanged);

    clearIntersections();
    calculateSceneRect();
}

void ScreenPositionWidget::removeDevice(const QUuid &uuid)
{
    for (ScreenRectItem *item: qAsConst(items)) {
        if (item->uuid() == uuid) {
            disconnect(item, &ScreenRectItem::movementHasStarted, this, &ScreenPositionWidget::clearIntersections);
            disconnect(item, &ScreenRectItem::movementHasFinished, this, &ScreenPositionWidget::onScreenPositionChanged);
            items.removeOne(item);
            item->deleteLater();
            break;
        }
    }

    clearIntersections();
    calculateSceneRect();
}

void ScreenPositionWidget::clearWidget()
{
    for (ScreenRectItem *item: qAsConst(items)) {
        item->disconnect();
        item->deleteLater();
    }
    items.clear();
}

void ScreenPositionWidget::calculateSceneRect()
{
    if (items.isEmpty())
        return;

    ScreenRectItem *item = items.at(0);
    int x = item->x(), y = item->y();
    int w = item->x() + item->width();
    int h = item->y() + item->height();
    int indent = item->height();

    for (int i=1; i<items.size(); ++i) {
        const QRect &rect = items.at(i)->geometry();
        if (rect.x() < x) x = rect.x();
        if (rect.y() < y) y = rect.y();
        if (rect.width() + rect.x() > w) w = rect.width() + rect.x();
        if (rect.height() + rect.y() > h) h = rect.height() + rect.y();
    }

    QRectF bound(QPointF(x - indent, y - indent),
                 QPointF(w + indent, h + indent));

    setSceneRect(bound);
    fitInView(bound, Qt::KeepAspectRatio);
    calculateIntersections();
}

void ScreenPositionWidget::calculateIntersections()
{
    for (ScreenRectItem *calcItem: qAsConst(items)) {
        for (ScreenRectItem *item: qAsConst(items)) {
            if (calcItem == item) continue;

            const QVector<QRect> &rects = item->rects();

            for (const QRect &rect: rects) {
                QRect itemRect(item->x() + rect.x(),
                               item->y() + rect.y(),
                               rect.width(), rect.height());

                calcItem->calculateTransits(item->uuid(), itemRect);
            }
        }
    }
}

void ScreenPositionWidget::clearIntersections()
{
    for (ScreenRectItem *calcItem: qAsConst(items))
        calcItem->clearTransits();
}

void ScreenPositionWidget::onScreenPositionChanged(ScreenRectItem *item)
{
    calculateSceneRect();

    if (!item)
        return;

    emit screenPositionChanged(item->uuid(), QPoint(item->x(), item->y()));
}

void ScreenPositionWidget::resizeEvent(QResizeEvent *)
{
    calculateSceneRect();
}
