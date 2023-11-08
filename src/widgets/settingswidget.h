#pragma once

#include <QListWidgetItem>
#include <QJsonObject>
#include <QWidget>
#include <QUuid>

#include "screenpositionwidget.h"
#include "deviceitemwidget.h"
#include "jsonloader.h"
#include "global.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsWidget; }
QT_END_NAMESPACE

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

    void initialize();
    void clearWidget();

public slots:
    void onDeviceConnectionChanged(const QUuid &uuid, Device::ConnectionState state);

signals:
    void findDevices();
    void removeDevice(const QUuid &uuid);
    void screenPositionChanged(const QUuid &uuid, const QPoint &pos);

private:
    Ui::SettingsWidget *ui;
    ScreenPositionWidget *positioningWidget = nullptr;
    QMap<QUuid, DeviceItemWidget*> deviceWidgets;
    QMap<QUuid, QListWidgetItem*> listIitemWidgets;
    JsonLoader geometryLoader;

    void createFoundDeviceWidget(QSharedPointer<Device> device);
    void removeDeviceFromListWidget(const QUuid &uuid);
    void onBtnOkClicked();

    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
};
