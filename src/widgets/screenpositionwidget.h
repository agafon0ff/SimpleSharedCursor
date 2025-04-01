#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QRect>
#include <QUuid>
#include "global.h"

#include "screenrectitem.h"

class ScreenPositionWidget : public QGraphicsView
{
    Q_OBJECT
public:
    ScreenPositionWidget(QWidget *parent = nullptr);
    ~ScreenPositionWidget();

    QVector<ScreenRectItem*> screenRectItems() const;

public slots:
    void addDevice(QSharedPointer<SharedCursor::Device> device);
    void removeDevice(const QUuid &uuid);
    void clearWidget();
    void normalize();

private slots:
    void onItemSelected(ScreenRectItem *item, bool state);
    void onItemPositionChanged(ScreenRectItem *item, const QPointF &dpos);
    void calculateSceneRect();
    void calculateTransits();

private:
    QVector<ScreenRectItem*> items;
    QGraphicsScene graphicsScene;
    QPointF minPos{0., 0.};

    ScreenRectItem *createScreen(const SharedCursor::Screen &screen, const QPoint &pos);
    void calculateTransitsTopBottom(ScreenRectItem *itemFrom, ScreenRectItem *itemTo);
    void calculateTransitsLeftRight(ScreenRectItem *itemFrom, ScreenRectItem *itemTo);

    void resizeEvent(QResizeEvent *) override;
};
