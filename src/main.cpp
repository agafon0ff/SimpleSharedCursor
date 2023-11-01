#include "broadcastdevicesearch.h"
#include "deviceconnectmanager.h"
#include "settingswidget.h"
#include "settingsloader.h"
#include "cursorhandler.h"
#include "global.h"
#include "utils.h"

#include <QGuiApplication>
#include <QApplication>
#include <QTranslator>
#include <QThreadPool>
#include <QJsonArray>
#include <QHostInfo>
#include <QLocale>
#include <QThread>
#include <QScreen>
#include <QUuid>

#include <QDebug>

void saveMyself() {
    QString strUuid = Settings.value(KEY_UUID).toString();

    if (strUuid.isEmpty()) {
        strUuid = QUuid::createUuid().toString();
        Settings.setValue(KEY_UUID, strUuid);
    }

    QJsonObject devices = Settings.value(KEY_DEVICES).toObject();
    if (devices.contains(strUuid))
        return;

    QJsonObject device;
    device.insert(KEY_UUID, QUuid::createUuid().toString());
    device.insert(KEY_NAME, QHostInfo::localHostName());
    device.insert(KEY_HOST, "127.0.0.1");
    device.insert(KEY_SELF, true);
    device.insert(KEY_UUID, strUuid);

    QVector<QRect> resolutions;
    QList <QScreen*> screens = QGuiApplication::screens();
    for (QScreen *screen: qAsConst(screens))
        resolutions.append(screen->geometry());

    device.insert(KEY_SCREENS, Utils::rectListToJsonValue(resolutions));
    device.insert(KEY_POSITION, Utils::pointToJsonValue(QPoint(0, 0)));

    devices.insert(strUuid, device);
    Settings.setValue(KEY_DEVICES, devices);
}

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

    saveMyself();

    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    CursorHandler cursorHandler;
    QThread cursorCheckerThread;
    QObject::connect(&cursorCheckerThread, &QThread::started, &cursorHandler, &CursorHandler::start);
    QObject::connect(&cursorCheckerThread, &QThread::finished, &cursorHandler, &CursorHandler::stop);
    cursorHandler.moveToThread(&cursorCheckerThread);

    DeviceConnectManager devConnectManager;
    QThread devConnectManagerThread;
    QObject::connect(&devConnectManagerThread, &QThread::started, &devConnectManager, &DeviceConnectManager::start);
    QObject::connect(&devConnectManagerThread, &QThread::finished, &devConnectManager, &DeviceConnectManager::stop);
    devConnectManager.moveToThread(&devConnectManagerThread);

    SettingsWidget settingsWidget;
    settingsWidget.show();

    BroadcastDeviceSearch deviceSearch;
    deviceSearch.start();

    QObject::connect(&settingsWidget, &SettingsWidget::findDevices, &deviceSearch, &BroadcastDeviceSearch::search);
    QObject::connect(&deviceSearch, &BroadcastDeviceSearch::deviceFound, &devConnectManager, &DeviceConnectManager::handleDeviceFound);
    QObject::connect(&devConnectManager, &DeviceConnectManager::deviceChanged, &settingsWidget, &SettingsWidget::onDeviceChanged);
    QObject::connect(&settingsWidget, &SettingsWidget::removeDevice, &devConnectManager, &DeviceConnectManager::handleRemoveDevice);
    QObject::connect(&settingsWidget, &SettingsWidget::screenPositionChanged, &devConnectManager, &DeviceConnectManager::handleScreenPositionChanged);

    cursorCheckerThread.start();
    devConnectManagerThread.start();

    int result = a.exec();

    devConnectManagerThread.quit();
    cursorCheckerThread.quit();

    return result;
}
