#include <QMessageBox>
#include <QCloseEvent>
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

    connect(ui->btnFindDevices, &QPushButton::clicked, this, &SettingsWidget::findDevices);
    connect(ui->btnOk, &QPushButton::clicked, this, &SettingsWidget::onBtnOkClicked);

    positioningWidget = new ScreenPositionWidget(this);
    ui->positioningLayout->addWidget(positioningWidget, 0, 0);
}

SettingsWidget::~SettingsWidget()
{
    clearWidget();
    delete ui;
}

void SettingsWidget::initialize()
{
    qDebug() << Q_FUNC_INFO;

    geometryLoader.load(ShareCursor::GEOMETRY_PATH);
    restoreGeometry(QByteArray::fromBase64(geometryLoader.value(ShareCursor::KEY_GEOMETRY).toString().toUtf8()));
    ui->splitter->restoreState(QByteArray::fromBase64(geometryLoader.value(ShareCursor::KEY_SPLITTER).toString().toUtf8()));

    ui->lineEditKeyword->setText(Settings.keyword());
    ui->lineDeviceName->setText(Settings.name());

    const QMap<QUuid, QSharedPointer<ShareCursor::Device>> &devices = Settings.devices();
    auto i = devices.constBegin();
    while (i != devices.constEnd()) {
        createFoundDeviceWidget(i.value());
        ++i;
    }
}

void SettingsWidget::clearWidget()
{
    qDebug() << Q_FUNC_INFO;

    auto itemTerator = listIitemWidgets.constBegin();
    while (itemTerator != listIitemWidgets.constEnd()) {
        delete itemTerator.value();
        ++itemTerator;
    }

    auto deviceIterator = deviceWidgets.constBegin();
    while (deviceIterator != deviceWidgets.constEnd()) {
        deviceIterator.value()->disconnect();
        delete deviceIterator.value();
        ++deviceIterator;
    }

    ui->listWidgetDevices->clear();
    listIitemWidgets.clear();
    deviceWidgets.clear();

    positioningWidget->clearWidget();

    geometryLoader.setValue(ShareCursor::KEY_GEOMETRY, QString::fromUtf8(saveGeometry().toBase64()));
    geometryLoader.setValue(ShareCursor::KEY_SPLITTER, QString::fromUtf8(ui->splitter->saveState().toBase64()));
    geometryLoader.save(ShareCursor::GEOMETRY_PATH);
}

void SettingsWidget::setDeviceConnectionState(const QUuid &uuid, ShareCursor::ConnectionState state)
{
    qDebug() << Q_FUNC_INFO << uuid << state;

    if (deviceWidgets.contains(uuid)) {
        deviceWidgets.value(uuid)->setState(state);
    }
    else {
        createFoundDeviceWidget(Settings.device(uuid));
    }
}

void SettingsWidget::createFoundDeviceWidget(QSharedPointer<ShareCursor::Device> device)
{
    if (device.isNull())
        return;

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

void SettingsWidget::onBtnOkClicked()
{
    if (Settings.name() != ui->lineDeviceName->text()) {
        Settings.setName(ui->lineDeviceName->text());
        emit nameChanged(Settings.name());
    }

    if (Settings.keyword() != ui->lineEditKeyword->text()) {
        Settings.setKeyword(ui->lineEditKeyword->text());
        emit nameChanged(Settings.keyword());
    }

    const QVector<ScreenRectItem*> &items = positioningWidget->screenRectItems();
    for (ScreenRectItem* item: items) {
        Settings.setDevicePosition(item->uuid(), item->position());
        Settings.setTransitsToDevice(item->uuid(), item->transits());
    }

    emit transitsChanged(Settings.transits());
    Settings.save();
}

void SettingsWidget::showEvent(QShowEvent *)
{
    initialize();
}

void SettingsWidget::closeEvent(QCloseEvent *e)
{
    e->ignore();
    hide();
    clearWidget();
}
