#include <QCursor>

#include "inputsimulator.h"

#ifdef WIN32
#endif

#ifdef __linux__
#endif

InputSimulator::InputSimulator(QObject *parent)
    : QObject{parent}
{

}

void InputSimulator::setCutsorPosition(const QPoint &pos)
{
    QCursor::setPos(pos);
}

void InputSimulator::setCutsorDelta(const QPoint &pos)
{
    QCursor::setPos(QCursor::pos() + pos);
}
