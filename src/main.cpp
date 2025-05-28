#include "broadcastdevicesearch.h"
#include "deviceconnectmanager.h"
#include "clipboardhandler.h"
#include "settingswidget.h"
#include "settingsfacade.h"
#include "inputsimulator.h"
#include "cursorhandler.h"
#include "inputhandler.h"
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
    qRegisterMetaType<SharedCursor::Device>("SharedCursor::Device");
    qRegisterMetaType<SharedCursor::ConnectionState>("SharedCursor::ConnectionState");
    qRegisterMetaType<QSharedPointer<SharedCursor::Device>>("QSharedPointer<SharedCursor::Device>");
    qRegisterMetaType<QMap<QUuid,QVector<SharedCursor::Transit>> >("QMap<QUuid,QVector<SharedCursor::Transit> >");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SimpleSharedCursor_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    Settings.loadFacadeProperties();

    ClipboardHandler clipboardHandler;
    clipboardHandler.setCurrentUuid(Settings.uuid());

    InputHandler inputHandler;
    inputHandler.setUuid(Settings.uuid());
    inputHandler.setGeometry(Settings.screenRect().adjusted(150, 150, -150, -150));
    inputHandler.setCenterIn(Settings.screenRect().center());

    InputSimulator inputSimulator;

    CursorHandler cursorHandler;
    cursorHandler.setHoldCursorPosition(Settings.screenRect().center());

    QThread cursorCheckerThread;
    QObject::connect(&cursorCheckerThread, &QThread::started, &cursorHandler, &CursorHandler::start);
    QObject::connect(&cursorCheckerThread, &QThread::finished, &cursorHandler, &CursorHandler::stop);
    QObject::connect(&cursorHandler, &CursorHandler::remoteControl, &inputHandler, &InputHandler::setRemoteControlState);
    QObject::connect(&cursorHandler, &CursorHandler::controlStateChanged, &inputSimulator, &InputSimulator::setControlState);
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
    QObject::connect(&settingsWidget, &SettingsWidget::devicesChanged, &cursorHandler, &CursorHandler::setDevices);
    QObject::connect(&inputHandler, &InputHandler::message, &devConnectManager, &DeviceConnectManager::sendMessage);
    QObject::connect(&cursorHandler, &CursorHandler::message, &devConnectManager, &DeviceConnectManager::sendMessage);
    QObject::connect(&cursorHandler, &CursorHandler::remoteControl, &devConnectManager, &DeviceConnectManager::sendRemoteControlMessage);
    QObject::connect(&cursorHandler, &CursorHandler::remoteControl, &inputHandler, &InputHandler::setRemoteControlState);
    QObject::connect(&cursorHandler, &CursorHandler::remoteControl, &clipboardHandler, &ClipboardHandler::setRemoteControlState);
    QObject::connect(&devConnectManager, &DeviceConnectManager::remoteControl, &cursorHandler, &CursorHandler::setRemoteControlState);
    QObject::connect(&devConnectManager, &DeviceConnectManager::remoteControl, &inputHandler, &InputHandler::setRemoteControlState);
    QObject::connect(&devConnectManager, &DeviceConnectManager::cursorPosition, &cursorHandler, &CursorHandler::setRemoteCursorPos);
    QObject::connect(&devConnectManager, &DeviceConnectManager::cursorDelta, &cursorHandler, &CursorHandler::setRemoteCursorDelta);
    QObject::connect(&devConnectManager, &DeviceConnectManager::cursorInitPosition, &inputSimulator, &InputSimulator::setCutsorPosition);
    QObject::connect(&devConnectManager, &DeviceConnectManager::cursorDelta, &inputSimulator, &InputSimulator::setCursorDelta);
    QObject::connect(&devConnectManager, &DeviceConnectManager::keyboardEvent, &inputSimulator, &InputSimulator::setKeyboardEvent);
    QObject::connect(&devConnectManager, &DeviceConnectManager::mouseEvent, &inputSimulator, &InputSimulator::setMouseEvent);
    QObject::connect(&devConnectManager, &DeviceConnectManager::wheelEvent, &inputSimulator, &InputSimulator::setWheelEvent);
    QObject::connect(&devConnectManager, &DeviceConnectManager::remoteControl, &clipboardHandler, &ClipboardHandler::setRemoteControlState);
    QObject::connect(&devConnectManager, &DeviceConnectManager::clipboard, &clipboardHandler, &ClipboardHandler::setClipboard);
    QObject::connect(&clipboardHandler, &ClipboardHandler::message, &devConnectManager, &DeviceConnectManager::sendMessage);

    cursorCheckerThread.start();
    devConnectManagerThread.start();

    Settings.loadDevices();
    cursorHandler.setCurrentUuid(Settings.uuid());
    cursorHandler.setDevices(Settings.devices());

    int result = a.exec();

    devConnectManagerThread.quit();
    cursorCheckerThread.quit();

    return result;
}
