#pragma once

#include <QSharedPointer>
#include <QHostAddress>
#include <QWidget>
#include <QUuid>
#include "global.h"

namespace Ui {
class DeviceItemWidget;
}

class DeviceItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceItemWidget(QWidget *parent = nullptr);
    ~DeviceItemWidget();

    void setUuid(const QUuid& uuid);
    QUuid getUuid() const;

    void setName(const QString& name);
    void setHost(const QHostAddress& host);
    void setState(SharedCursor::ConnectionState state);

signals:
    void removeClicked(const QUuid &uuid);

private:
    Ui::DeviceItemWidget *ui;
    QPixmap pixmapConnected, pixmapDisconnected;
    QUuid uuid;
    SharedCursor::ConnectionState state = SharedCursor::Unknown;
};
