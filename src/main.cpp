#include "broadcastdevicesearch.h"
#include "deviceconnectmanager.h"
#include "settingswidget.h"
#include "settingsfacade.h"
#include "cursorhandler.h"
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

    qRegisterMetaType<Device>("Device");
    qRegisterMetaType<QSharedPointer<Device>>("QSharedPointer<Device>");

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
    settingsWidget.show();

    BroadcastDeviceSearch deviceSearch;
    deviceSearch.setPort(Settings.portUdp());
    deviceSearch.setUuid(Settings.uuid());
    deviceSearch.setKeyword(Settings.keyword());
    deviceSearch.start();

    QObject::connect(&settingsWidget, &SettingsWidget::findDevices, &deviceSearch, &BroadcastDeviceSearch::search);
    QObject::connect(&deviceSearch, &BroadcastDeviceSearch::deviceFound, &devConnectManager, &DeviceConnectManager::handleDeviceFound);
    QObject::connect(&deviceSearch, &BroadcastDeviceSearch::deviceFound, &Settings, &SettingsFacade::setDevice);
    QObject::connect(&devConnectManager, &DeviceConnectManager::deviceConnectionChanged, &settingsWidget, &SettingsWidget::onDeviceConnectionChanged);
    QObject::connect(&settingsWidget, &SettingsWidget::removeDevice, &devConnectManager, &DeviceConnectManager::handleRemoveDevice);

    cursorCheckerThread.start();
    devConnectManagerThread.start();

    int result = a.exec();

    devConnectManagerThread.quit();
    cursorCheckerThread.quit();

    return result;
}
