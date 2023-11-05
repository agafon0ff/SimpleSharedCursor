#include <QCommonStyle>

#include "traymenu.h"
#include "global.h"

TrayMenu::TrayMenu(QObject *parent)
    : QObject{parent}
{
    QCommonStyle style;
    trayMenu.addAction(QIcon(style.standardPixmap(QStyle::SP_MessageBoxInformation)),"Help");
    trayMenu.addAction(QIcon(style.standardPixmap(QStyle::SP_DialogCancelButton)),"Exit");

    trayIcon.setContextMenu(&trayMenu);
    trayIcon.setIcon(QIcon("://img/ShareCursor.ico"));
    trayIcon.setToolTip(PROGRAM_NAME);
    trayIcon.show();
}
