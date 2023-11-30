#pragma once

#include <QObject>

class InputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit InputSimulator(QObject *parent = nullptr);

public slots:
    void setCutsorPosition(const QPoint &pos);
    void setCutsorDelta(const QPoint &pos);

signals:

};
