#include <QMessageBox>
#include <QHostInfo>
#include <QMap>

#include "ui_settingswidget.h"

#include "settingswidget.h"
#include "settingsfacade.h"
#include "utils.h"

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    initialize();

    connect(ui->btnFindDevices, &QPushButton::clicked, this, &SettingsWidget::findDevices);
    connect(ui->btnOk, &QPushButton::clicked, this, &SettingsWidget::onBtnOkClicked);
}

SettingsWidget::~SettingsWidget()
{
    Settings.setValue(KEY_GEOMETRY, QString::fromUtf8(saveGeometry().toBase64()));
    Settings.setValue(KEY_SPLITTER, QString::fromUtf8(ui->splitter->saveState().toBase64()));
    Settings.setValue(KEY_NAME, ui->lineDeviceName->text());
    Settings.setValue(KEY_KEYWORD, ui->lineEditKeyword->text());

    delete ui;
}

void SettingsWidget::initialize()
{
    const QString &deviceName = Settings.value(KEY_NAME, QHostInfo::localHostName()).toString();

    restoreGeometry(QByteArray::fromBase64(Settings.value(KEY_GEOMETRY).toString().toUtf8()));
    ui->splitter->restoreState(QByteArray::fromBase64(Settings.value(KEY_SPLITTER).toString().toUtf8()));
    ui->lineEditKeyword->setText(Settings.value(KEY_KEYWORD).toString());
    ui->lineDeviceName->setText(deviceName);

    positioningWidget = new ScreenPositionWidget(this);
    ui->positioningLayout->addWidget(positioningWidget, 0, 0);

    connect(positioningWidget, &ScreenPositionWidget::screenPositionChanged, this, &SettingsWidget::onScreenPositionChanged);
}

void SettingsWidget::onDeviceChanged(QSharedPointer<Device> device)
{
    qDebug() << Q_FUNC_INFO << device->name << device->state;

    if (deviceWidgets.contains(device->uuid)) {
        deviceWidgets.value(device->uuid)->setState(device->state);
    }
    else {
        createFoundDeviceWidget(device);
    }
}

void SettingsWidget::createFoundDeviceWidget(QSharedPointer<Device> device)
{
    if (!device->self) {
        DeviceItemWidget *widget = new DeviceItemWidget(this);
        widget->setUuid(device->uuid);
        widget->setName(device->name);
        widget->setHost(device->host);
        widget->setState(device->state);

        deviceWidgets.insert(device->uuid, widget);

        QListWidgetItem *listItem = new QListWidgetItem(ui->listWidgetDevices);
        listItem->setSizeHint(widget->sizeHint());

        listIitemWidgets.insert(device->uuid, listItem);
        ui->listWidgetDevices->addItem(listItem);
        ui->listWidgetDevices->setItemWidget(listItem, widget);

        connect(widget, &DeviceItemWidget::removeClicked, this, &SettingsWidget::removeDeviceFromListWidget);
    }

    positioningWidget->addDevice(device);
}

void SettingsWidget::removeDeviceFromListWidget(const QUuid &uuid)
{
    if (listIitemWidgets.contains(uuid)) {
        DeviceItemWidget *deviceWidget = deviceWidgets.value(uuid);
        QListWidgetItem *listItem = listIitemWidgets.value(uuid);

        ui->listWidgetDevices->removeItemWidget(listItem);
        listIitemWidgets.remove(uuid);
        deviceWidgets.remove(uuid);

        disconnect(deviceWidget, &DeviceItemWidget::removeClicked, this, &SettingsWidget::removeDeviceFromListWidget);

        delete listItem;
        delete deviceWidget;
    }

    positioningWidget->removeDevice(uuid);

    emit removeDevice(uuid);
}

void SettingsWidget::onScreenPositionChanged(const QUuid &uuid, const QPoint &pos)
{

}

void SettingsWidget::onBtnOkClicked()
{

}
