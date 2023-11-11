#include "broadcastdevicesearch.h"
#include "deviceconnectmanager.h"
#include "settingswidget.h"
#include "settingsfacade.h"
#include "cursorhandler.h"
#include "traymenu.h"
#include "global.h"

#include <QApplication>
#include <QTranslator>
#include <QThreadPool>
#include <QLocale>
#include <QThread>
#include <QUuid>

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<ShareCursor::Device>("ShareCursor::Device");
    qRegisterMetaType<ShareCursor::ConnectionState>("ShareCursor::ConnectionState");
    qRegisterMetaType<QSharedPointer<ShareCursor::Device>>("QSharedPointer<ShareCursor::Device>");
    qRegisterMetaType<QMap<QUuid,QVector<ShareCursor::Transit>> >("QMap<QUuid,QVector<ShareCursor::Transit> >");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ShareCursor_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    Settings.loadFacadeProperties();

    CursorHandler cursorHandler;
    QThread cursorCheckerThread;
    QObject::connect(&cursorCheckerThread, &QThread::started, &cursorHandler, &CursorHandler::start);
    QObject::connect(&cursorCheckerThread, &QThread::finished, &cursorHandler, &CursorHandler::stop);
    cursorHandler.moveToThread(&cursorCheckerThread);

    DeviceConnectManager devConnectManager;
    devConnectManager.setPort(Settings.portTcp());
    devConnectManager.setUuid(Settings.uuid());
    devConnectManager.setKeyword(Settings.keyword());

    QThread devConnectManagerThread;
    QObject::connect(&devConnectManagerThread, &QThread::started, &devConnectManager, &DeviceConnectManager::start);
    QObject::connect(&devConnectManagerThread, &QThread::finished, &devConnectManager, &DeviceConnectManager::stop);
    devConnectManager.moveToThread(&devConnectManagerThread);

    SettingsWidget settingsWidget;
    TrayMenu trayMenu;
    QObject::connect(&trayMenu, &TrayMenu::settingsActionTriggered, &settingsWidget, &SettingsWidget::show);

    BroadcastDeviceSearch deviceSearch;
    deviceSearch.setPort(Settings.portUdp());
    deviceSearch.setUuid(Settings.uuid());
    deviceSearch.setKeyword(Settings.keyword());
    deviceSearch.start();

    QObject::connect(&settingsWidget, &SettingsWidget::findDevices, &deviceSearch, &BroadcastDeviceSearch::search);
    QObject::connect(&settingsWidget, &SettingsWidget::keywordChanged, &deviceSearch, &BroadcastDeviceSearch::setKeyword);
    QObject::connect(&deviceSearch, &BroadcastDeviceSearch::deviceFound, &Settings, &SettingsFacade::setDevice);
    QObject::connect(&Settings, &SettingsFacade::deviceFound, &devConnectManager, &DeviceConnectManager::connectToDevice);
    QObject::connect(&devConnectManager, &DeviceConnectManager::deviceConnectionChanged, &Settings, &SettingsFacade::setDeviceConnectionState);
    QObject::connect(&devConnectManager, &DeviceConnectManager::deviceConnectionChanged, &settingsWidget, &SettingsWidget::setDeviceConnectionState);
    QObject::connect(&devConnectManager, &DeviceConnectManager::deviceConnectionChanged, &cursorHandler, &CursorHandler::setConnectionState);
    QObject::connect(&settingsWidget, &SettingsWidget::removeDevice, &devConnectManager, &DeviceConnectManager::handleRemoveDevice);
    QObject::connect(&settingsWidget, &SettingsWidget::keywordChanged, &devConnectManager, &DeviceConnectManager::setKeyword);
    QObject::connect(&settingsWidget, &SettingsWidget::transitsChanged, &cursorHandler, &CursorHandler::setTransits);

    cursorCheckerThread.start();
    devConnectManagerThread.start();

    Settings.loadDevices();
    cursorHandler.setCurrentUuid(Settings.uuid());
    cursorHandler.setTransits(Settings.transits());

    settingsWidget.show();

    int result = a.exec();

    devConnectManagerThread.quit();
    cursorCheckerThread.quit();

    return result;
}
