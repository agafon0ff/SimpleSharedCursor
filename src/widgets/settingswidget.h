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
    void setDeviceConnectionState(const QUuid &uuid, ShareCursor::ConnectionState state);

signals:
    void findDevices();
    void nameChanged(const QString &name);
    void keywordChanged(const QString &key);
    void removeDevice(const QUuid &uuid);
    void screenPositionChanged(const QUuid &uuid, const QPoint &pos);
    void transitsChanged(const QMap<QUuid, QVector<ShareCursor::Transit>> &transits);

private:
    Ui::SettingsWidget *ui;
    ScreenPositionWidget *positioningWidget = nullptr;
    QMap<QUuid, DeviceItemWidget*> deviceWidgets;
    QMap<QUuid, QListWidgetItem*> listIitemWidgets;
    JsonLoader geometryLoader;
    QVector<QUuid> removeList;

    void createFoundDeviceWidget(QSharedPointer<ShareCursor::Device> device);
    void removeDeviceFromListWidget(const QUuid &uuid);
    void onBtnOkClicked();
    void onBtnCancelClicked();

    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
};
