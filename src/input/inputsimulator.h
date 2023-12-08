#pragma once

#include <QObject>
#include <QMap>

class InputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit InputSimulator(QObject *parent = nullptr);

public slots:
    void setCutsorPosition(const QPoint &pos);
    void setCutsorDelta(const QPoint &pos);
    void setKeyboardEvent(int keycode, bool state);
    void setMouseEvent(int button, bool state);
    void setWheelEvent(int delta);

private:
    QMap<int, unsigned long> keymap;
    void createKeymap();
};
