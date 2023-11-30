QT += core gui widgets network

CONFIG += c++17
INCLUDEPATH += \
    src \
    src/network \
    src/settings \
    src/widgets

SOURCES += \
    src/cursorhandler.cpp \
    src/inputsimulator.cpp \
    src/main.cpp \
    src/network/deviceconnectmanager.cpp \
    src/network/tcpsocket.cpp \
    src/settings/jsonloader.cpp \
    src/settings/settingsfacade.cpp \
    src/opensslwrapper.cpp \
    src/network/broadcastdevicesearch.cpp \
    src/network/tcpserver.cpp \
    src/widgets/cursorholder.cpp \
    src/widgets/deviceitemwidget.cpp \
    src/widgets/screenpositionwidget.cpp \
    src/widgets/screenrectitem.cpp \
    src/widgets/settingswidget.cpp \
    src/widgets/traymenu.cpp

HEADERS += \
    src/cursorhandler.h \
    src/global.h \
    src/inputsimulator.h \
    src/network/deviceconnectmanager.h \
    src/network/tcpsocket.h \
    src/settings/jsonloader.h \
    src/settings/settingsfacade.h \
    src/opensslwrapper.h \
    src/network/broadcastdevicesearch.h \
    src/network/tcpserver.h \
    src/utils.h \
    src/widgets/cursorholder.h \
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

CONFIG += lrelease
CONFIG += embed_translations

LIBS += -lcrypto

