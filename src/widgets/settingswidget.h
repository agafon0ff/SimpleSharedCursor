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
    void setDeviceConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state);

signals:
    void findDevices();
    void nameChanged(const QString &name);
    void keywordChanged(const QString &key);
    void removeDevice(const QUuid &uuid);
    void screenPositionChanged(const QUuid &uuid, const QPoint &pos);
    void devicesChanged(const QMap<QUuid, QSharedPointer<SharedCursor::Device>> &devices);

private:
    Ui::SettingsWidget *ui;
    ScreenPositionWidget *_positioningWidget = nullptr;
    QMap<QUuid, DeviceItemWidget*> _deviceWidgets;
    QMap<QUuid, QListWidgetItem*> _listIitemWidgets;
    JsonLoader _geometryLoader;
    QVector<QUuid> _removeList;

    void createFoundDeviceWidget(QSharedPointer<SharedCursor::Device> device);
    void removeDeviceFromListWidget(const QUuid &uuid);
    void saveSettings();

    void onBtnFindDevicesClicked();
    void onBtnOkClicked();
    void onBtnCancelClicked();
    void onBtnApplyClicked();

    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
};
