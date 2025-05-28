#include <QCommonStyle>
#include <QApplication>

#include "traymenu.h"
#include "global.h"

TrayMenu::TrayMenu(QObject *parent)
    : QObject{parent}
{
    QCommonStyle style;
    _trayMenu.addAction(QIcon("://img/gear.png"),"Settings", this, &TrayMenu::settingsActionTriggered);
    _trayMenu.addAction(QIcon("://img/cross.png"),"Exit", this, []{QApplication::quit();});

    _trayIcon.setContextMenu(&_trayMenu);
    _trayIcon.setIcon(QIcon("://img/SharedCursor.ico"));
    _trayIcon.setToolTip(SharedCursor::PROGRAM_NAME);
    _trayIcon.show();
}
