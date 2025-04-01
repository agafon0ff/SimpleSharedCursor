#pragma once

#include <QSharedPointer>
#include <QHostAddress>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QUuid>
#include "global.h"

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
    void setSelfState(bool self);

signals:
    void removeClicked(const QUuid &uuid);

private:
    void paintEvent(QPaintEvent *) override;

    QHBoxLayout horizontalLayout;
    QLabel labelStatus;
    QLabel labelName;
    QLabel labelHost;
    QPushButton btnRemove;

    QPixmap pixmapConnected, pixmapDisconnected;
    QUuid uuid;
    SharedCursor::ConnectionState state = SharedCursor::Unknown;
    bool selfState = false;
};
