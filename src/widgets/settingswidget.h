#pragma once

#include <QListWidgetItem>
#include <QJsonObject>
#include <QWidget>
#include <QUuid>

#include "screenpositionwidget.h"
#include "deviceitemwidget.h"
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

public slots:
    void onDeviceChanged(QSharedPointer<Device> device);

signals:
    void findDevices();
    void removeDevice(const QUuid &uuid);
    void screenPositionChanged(const QUuid &uuid, const QPoint &pos);

private:
    Ui::SettingsWidget *ui;
    ScreenPositionWidget *positioningWidget = nullptr;
    QMap<QUuid, DeviceItemWidget*> deviceWidgets;
    QMap<QUuid, QListWidgetItem*> listIitemWidgets;

    void createFoundDeviceWidget(QSharedPointer<Device> device);
    void removeDeviceFromListWidget(const QUuid &uuid);
    void onScreenPositionChanged(const QUuid &uuid, const QPoint &pos);
    void onBtnOkClicked();
};
