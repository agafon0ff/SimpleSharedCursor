#include "inputsimulator.h"

#if defined(Q_OS_WINDOWS)
#include "win/inputsimulatorwindows.h"
#elif defined(Q_OS_LINUX)
#include "linux/inputsimulatorlinux.h"
#elif defined(Q_OS_MACOS)
#include "macos/inputsimulatormacos.h"
#endif

InputSimulator::InputSimulator(QObject *parent)
    : QObject(parent)
{
    qDebug() << Q_FUNC_INFO;
}

InputSimulator::~InputSimulator()
{

}

std::unique_ptr<InputSimulator> InputSimulator::create()
{
#if defined(Q_OS_WINDOWS)
    return std::make_unique<InputSimulatorWindows>();
#elif defined(Q_OS_LINUX)
    return std::make_unique<InputSimulatorLinux>();
#elif defined(Q_OS_MACOS)
    return std::make_unique<InputSimulatorMacos>();
#endif
}
