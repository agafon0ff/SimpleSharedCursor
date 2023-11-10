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
    void setState(ShareCursor::ConnectionState state);

signals:
    void removeClicked(const QUuid &uuid);

private:
    Ui::DeviceItemWidget *ui;
    QUuid uuid;
    ShareCursor::ConnectionState state = ShareCursor::Unknown;
};
