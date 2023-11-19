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

    void addTransit(ShareCursor::Transit transit);
    void clearTransits();
    QVector<ShareCursor::Transit> transits() const;

    qreal width() const;
    qreal height() const;
    QRectF adjusted();

signals:
    void positionChanged(ScreenRectItem *self, const QPointF &dpos);
    void selected(bool state);
    bool pressed();
    bool released();

private:
    QUuid _uuid;
    QFont _font;
    QString _text;
    bool _isSelected = false;
    QVector<ShareCursor::Transit> _transits;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};
