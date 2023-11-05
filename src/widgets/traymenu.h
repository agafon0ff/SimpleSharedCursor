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
    QSystemTrayIcon trayIcon;
    QMenu trayMenu;

signals:

};
