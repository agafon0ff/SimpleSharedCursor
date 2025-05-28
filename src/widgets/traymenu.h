#pragma once

#include <QSystemTrayIcon>
#include <QObject>
#include <QAction>
#include <QMenu>

class TrayMenu : public QObject
{
    Q_OBJECT
public:
    explicit TrayMenu(QObject *parent = nullptr);
    QSystemTrayIcon _trayIcon;
    QMenu _trayMenu;

signals:
    void settingsActionTriggered();
};
