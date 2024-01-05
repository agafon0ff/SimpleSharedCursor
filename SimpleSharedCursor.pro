QT += core gui widgets network

CONFIG += c++17
CONFIG += lrelease
CONFIG += embed_translations

TEMPLATE = app
RC_FILE = img/icon.rc

QMAKE_LFLAGS += -no-pie
QMAKE_CXXFLAGS_RELEASE += -O2

INCLUDEPATH += \
    src \
    src/input \
    src/network \
    src/settings \
    src/widgets

SOURCES += \
    src/main.cpp \
    src/opensslwrapper.cpp \
    src/input/clipboardhandler.cpp \
    src/input/cursorhandler.cpp \
    src/input/inputsimulator.cpp \
    src/input/inputhandler.cpp \
    src/network/broadcastdevicesearch.cpp \
    src/network/deviceconnectmanager.cpp \
    src/network/tcpserver.cpp \
    src/network/tcpsocket.cpp \
    src/settings/jsonloader.cpp \
    src/settings/settingsfacade.cpp \
    src/widgets/deviceitemwidget.cpp \
    src/widgets/screenpositionwidget.cpp \
    src/widgets/screenrectitem.cpp \
    src/widgets/settingswidget.cpp \
    src/widgets/traymenu.cpp

HEADERS += \
    src/global.h \
    src/utils.h \
    src/opensslwrapper.h \
    src/input/clipboardhandler.h \
    src/input/cursorhandler.h \
    src/input/inputsimulator.h \
    src/input/inputhandler.h \
    src/network/deviceconnectmanager.h \
    src/network/broadcastdevicesearch.h \
    src/network/tcpserver.h \
    src/network/tcpsocket.h \
    src/settings/jsonloader.h \
    src/settings/settingsfacade.h \
    src/widgets/deviceitemwidget.h \
    src/widgets/screenpositionwidget.h \
    src/widgets/screenrectitem.h \
    src/widgets/settingswidget.h \
    src/widgets/traymenu.h

FORMS += \
    src/widgets/deviceitemwidget.ui \
    src/widgets/settingswidget.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS += \
    tr/ShareCursor_en.ts


win32: \
    INCLUDEPATH += "C:/Program Files (x86)/OpenSSL-Win32/include"
    LIBS += "C:/Program Files (x86)/OpenSSL-Win32/bin/libcrypto-3.dll"

linux-g++: \
    LIBS += -lX11 -lXtst -lcrypto
