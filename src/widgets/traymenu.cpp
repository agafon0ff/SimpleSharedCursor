#include <QCommonStyle>
#include <QApplication>

#include "traymenu.h"
#include "global.h"

TrayMenu::TrayMenu(QObject *parent)
    : QObject{parent}
{
    QCommonStyle style;
    trayMenu.addAction(QIcon(style.standardPixmap(QStyle::SP_MessageBoxInformation)),"Settings",
                       this, &TrayMenu::settingsActionTriggered);
    trayMenu.addAction(QIcon(style.standardPixmap(QStyle::SP_DialogCancelButton)),"Exit", this, []{QApplication::quit();});

    trayIcon.setContextMenu(&trayMenu);
    trayIcon.setIcon(QIcon("://img/ShareCursor.ico"));
    trayIcon.setToolTip(SharedCursor::PROGRAM_NAME);
    trayIcon.show();
}
