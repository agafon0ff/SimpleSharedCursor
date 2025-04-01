#pragma once

#include <QGraphicsRectItem>
#include <QFont>
#include <QUuid>
#include "global.h"

class ScreenRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    explicit ScreenRectItem(QGraphicsItem *parent = nullptr);

    QUuid uuid() const;
    void setUuid(const QUuid &uuid);

    QString text() const;
    void setText(const QString &text);

    int index();
    void setIndex(int index);

    bool isEnabled();
    void setEnabled(bool state);

    void addTransit(SharedCursor::Transit transit);
    void clearTransits();
    QVector<SharedCursor::Transit> transits() const;

    qreal width() const;
    qreal height() const;
    QRectF adjusted();

signals:
    void positionChanged(ScreenRectItem *self, const QPointF &dpos);
    void selected(ScreenRectItem *self, bool state);
    void enabled(ScreenRectItem *self, bool state);
    void pressed(ScreenRectItem *self);
    void released(ScreenRectItem *self);

private:
    QUuid _uuid;
    QFont _font;
    QString _text;
    int _index = 0;
    bool _selected = false;
    bool _enabled = true;
    QVector<SharedCursor::Transit> _transits;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
};
