#include <QMessageBox>
#include <QCloseEvent>
#include <QHostInfo>
#include <QMap>

#include "ui_settingswidget.h"

#include "settingswidget.h"
#include "settingsfacade.h"

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    setWindowIcon(QIcon("://img/SharedCursor.ico"));

    connect(ui->btnFindDevices, &QPushButton::clicked, this, &SettingsWidget::onBtnFindDevicesClicked);
    connect(ui->btnOk, &QPushButton::clicked, this, &SettingsWidget::onBtnOkClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &SettingsWidget::onBtnCancelClicked);

    _positioningWidget = new ScreenPositionWidget(this);
    ui->positioningLayout->addWidget(_positioningWidget, 0, 0);
}

SettingsWidget::~SettingsWidget()
{
    clearWidget();
    delete ui;
}

void SettingsWidget::initialize()
{
    qDebug() << Q_FUNC_INFO;

    _geometryLoader.load(SharedCursor::GEOMETRY_PATH);
    restoreGeometry(QByteArray::fromBase64(_geometryLoader.value(SharedCursor::KEY_GEOMETRY).toString().toUtf8()));
    ui->splitter->restoreState(QByteArray::fromBase64(_geometryLoader.value(SharedCursor::KEY_SPLITTER).toString().toUtf8()));

    ui->lineEditKeyword->setText(Settings.keyword());
    ui->lineDeviceName->setText(Settings.name());

    Settings.loadDevices();
    const QMap<QUuid, QSharedPointer<SharedCursor::Device>> &devices = Settings.devices();
    for (auto i = devices.constBegin(); i != devices.constEnd(); ++i) {
        createFoundDeviceWidget(i.value());
    }
}

void SettingsWidget::clearWidget()
{
    qDebug() << Q_FUNC_INFO;

    auto itemTerator = _listIitemWidgets.constBegin();
    while (itemTerator != _listIitemWidgets.constEnd()) {
        delete itemTerator.value();
        ++itemTerator;
    }

    auto deviceIterator = _deviceWidgets.constBegin();
    while (deviceIterator != _deviceWidgets.constEnd()) {
        deviceIterator.value()->disconnect();
        delete deviceIterator.value();
        ++deviceIterator;
    }

    ui->listWidgetDevices->clear();
    _listIitemWidgets.clear();
    _deviceWidgets.clear();
    _removeList.clear();

    _positioningWidget->clearWidget();

    _geometryLoader.setValue(SharedCursor::KEY_GEOMETRY, QString::fromUtf8(saveGeometry().toBase64()));
    _geometryLoader.setValue(SharedCursor::KEY_SPLITTER, QString::fromUtf8(ui->splitter->saveState().toBase64()));
    _geometryLoader.save(SharedCursor::GEOMETRY_PATH);
}

void SettingsWidget::setDeviceConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state)
{
    qDebug() << Q_FUNC_INFO << uuid << state;

    if (_deviceWidgets.contains(uuid)) {
        _deviceWidgets.value(uuid)->setState(state);
    }
    else {
        createFoundDeviceWidget(Settings.device(uuid));
    }
}

void SettingsWidget::createFoundDeviceWidget(QSharedPointer<SharedCursor::Device> device)
{
    if (device.isNull())
        return;

    qDebug() << Q_FUNC_INFO << device->uuid << device->name << device->self;

    if (_deviceWidgets.contains(device->uuid))
        return;

    DeviceItemWidget *widget = new DeviceItemWidget(this);
    widget->setUuid(device->uuid);
    widget->setName(device->name);
    widget->setHost(device->host);
    widget->setState(device->state);
    widget->setSelfState(device->self);

    _deviceWidgets.insert(device->uuid, widget);

    QListWidgetItem *listItem = new QListWidgetItem;
    listItem->setSizeHint(widget->sizeHint());

    _listIitemWidgets.insert(device->uuid, listItem);

    ui->listWidgetDevices->insertItem((device->self ? 0 : ui->listWidgetDevices->count()), listItem);
    ui->listWidgetDevices->setItemWidget(listItem, widget);

    connect(widget, &DeviceItemWidget::removeClicked, this, &SettingsWidget::removeDeviceFromListWidget);

    _positioningWidget->addDevice(device);
}

void SettingsWidget::removeDeviceFromListWidget(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;

    if (_listIitemWidgets.contains(uuid)) {
        DeviceItemWidget *deviceWidget = _deviceWidgets.value(uuid);
        QListWidgetItem *listItem = _listIitemWidgets.value(uuid);

        ui->listWidgetDevices->removeItemWidget(listItem);
        _listIitemWidgets.remove(uuid);
        _deviceWidgets.remove(uuid);

        disconnect(deviceWidget, &DeviceItemWidget::removeClicked, this, &SettingsWidget::removeDeviceFromListWidget);

        delete listItem;
        delete deviceWidget;
    }

    _positioningWidget->removeDevice(uuid);
    _removeList.append(uuid);
    Settings.removeDevice(uuid);
}

void SettingsWidget::saveSettings()
{
    if (Settings.name() != ui->lineDeviceName->text()) {
        Settings.setName(ui->lineDeviceName->text());
        emit nameChanged(Settings.name());
    }

    if (Settings.keyword() != ui->lineEditKeyword->text()) {
        Settings.setKeyword(ui->lineEditKeyword->text());
        emit keywordChanged(Settings.keyword());
    }

    Settings.clearTransits();
    _positioningWidget->normalize();
    const QVector<ScreenRectItem*> &items = _positioningWidget->screenRectItems();
    for (ScreenRectItem* item: items) {
        Settings.setDevicePosition(item->uuid(), item->pos().toPoint());
        Settings.addTransitsToDevice(item->uuid(), item->transits());
        Settings.setScreenEnabled(item->uuid(), item->index(), item->isEnabled());
    }

    emit devicesChanged(Settings.devices());

    for (const QUuid &uuid: std::as_const(_removeList)) {
        Settings.removeDevice(uuid);
        emit removeDevice(uuid);
    }

    Settings.save();
}

void SettingsWidget::onBtnFindDevicesClicked()
{
    Settings.resetDevices();
    clearWidget();
    emit findDevices();

    createFoundDeviceWidget(Settings.device(Settings.uuid()));
}

void SettingsWidget::onBtnOkClicked()
{
    saveSettings();
    clearWidget();
    hide();
}

void SettingsWidget::onBtnCancelClicked()
{
    hide();
    clearWidget();
}

void SettingsWidget::onBtnApplyClicked()
{
    saveSettings();
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
