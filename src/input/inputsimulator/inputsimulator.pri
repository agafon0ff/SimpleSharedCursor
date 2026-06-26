INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/inputsimulator.h
SOURCES += $$PWD/inputsimulator.cpp

win32{
    HEADERS += $$PWD/win/inputsimulatorwindows.h
    SOURCES += $$PWD/win/inputsimulatorwindows.cpp
}

linux{
    HEADERS += $$PWD/linux/inputsimulatorlinux.h
    SOURCES += $$PWD/linux/inputsimulatorlinux.cpp
}

osx{
    HEADERS += $$PWD/macos/inputsimulatormacos.h
    SOURCES += $$PWD/macos/inputsimulatormacos.cpp
}
