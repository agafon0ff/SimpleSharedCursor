#pragma once

#include <QGraphicsView>
#include "screenrectitem.h"
#include "global.h"

class ScreenPositionWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ScreenPositionWidget(QWidget *parent = nullptr);
    ~ScreenPositionWidget();

    QVector<ScreenRectItem*> screenRectItems() const;

signals:
    void screenPositionChanged(const QUuid &uuid, const QPoint &pos);

public slots:
    void addDevice(QSharedPointer<Device> device);
    void removeDevice(const QUuid &uuid);
    void clearWidget();

private slots:
    void calculateSceneRect();
    void calculateIntersections();
    void clearIntersections();
    void onScreenPositionChanged(ScreenRectItem *item);

private:
    QVector<ScreenRectItem*> items;
    QGraphicsScene graphicsScene;

    void resizeEvent(QResizeEvent *) override;
};
