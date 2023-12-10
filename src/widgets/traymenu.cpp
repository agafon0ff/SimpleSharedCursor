#include <QCommonStyle>
#include <QApplication>

#include "traymenu.h"
#include "global.h"

TrayMenu::TrayMenu(QObject *parent)
    : QObject{parent}
{
    QCommonStyle style;
    trayMenu.addAction(QIcon("://img/gear.png"),"Settings", this, &TrayMenu::settingsActionTriggered);
    trayMenu.addAction(QIcon("://img/cross.png"),"Exit", this, []{QApplication::quit();});

    trayIcon.setContextMenu(&trayMenu);
    trayIcon.setIcon(QIcon("://img/SharedCursor.ico"));
    trayIcon.setToolTip(SharedCursor::PROGRAM_NAME);
    trayIcon.show();
}
