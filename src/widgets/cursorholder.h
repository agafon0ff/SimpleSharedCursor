#pragma once

#include <QWidget>

class CursorHolder : public QWidget
{
    Q_OBJECT
public:
    explicit CursorHolder(QWidget *parent = nullptr);

signals:

public slots:
    void holdCursor(bool state);

private:
    void paintEvent(QPaintEvent *) override;


};
