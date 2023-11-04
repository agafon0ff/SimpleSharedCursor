#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QPair>
#include <QUuid>

class ScreenRectItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit ScreenRectItem(QGraphicsItem *parent = nullptr);

    void setUuid(const QUuid &uuid);
    QUuid uuid() const;

    QPoint position();

    void setRects(const QVector<QRect> &rects);
    QVector<QRect> rects() const;

    void setGeometry(const QRect &rect);
    QRect geometry() const;

    int width() const;
    int height() const;

    void setBackgroundColor(const QColor &color);
    void setText(const QString &text);

    void calculateIntersection(const QUuid &uuid, const QRect& rect);
    void clearIntersections();

    QVector<QPair<QLine, QUuid>> intersects() const;

signals:
    void movementHasStarted(ScreenRectItem *self);
    void movementHasFinished(ScreenRectItem *self);

private:
    QUuid _uuid;
    QString _text;
    QSizeF _size {100., 100.};
    QVector<QRect> _rects;
    QVector<QPair<QLine, QUuid>> _intersects;
    bool isPositionChanged = false;
    QColor backgroundColor {160, 160, 160};

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

