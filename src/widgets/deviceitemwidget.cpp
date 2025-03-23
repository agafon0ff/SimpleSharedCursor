#include "deviceitemwidget.h"

DeviceItemWidget::DeviceItemWidget(QWidget *parent) :
    QWidget(parent),
    pixmapConnected{"://img/connected.png"},
    pixmapDisconnected{"://img/disconnected.png"}
{
    setMinimumHeight(30);
    setMaximumHeight(30);

    setLayout(&horizontalLayout);
    horizontalLayout.setSpacing(0);
    horizontalLayout.setContentsMargins(0, 0, 0, 0);

    labelStatus.setMinimumSize(QSize(30, 30));
    labelStatus.setMaximumSize(QSize(30, 30));
    labelStatus.setPixmap(QPixmap(":/img/disconnected.png"));
    labelStatus.setScaledContents(true);
    labelStatus.setAlignment(Qt::AlignCenter);
    labelStatus.setMargin(3);
    horizontalLayout.addWidget(&labelStatus);

    labelName.setObjectName(QString::fromUtf8("labelName"));
    labelName.setAlignment(Qt::AlignCenter);
    horizontalLayout.addWidget(&labelName);

    labelHost.setObjectName(QString::fromUtf8("labelHost"));
    labelHost.setAlignment(Qt::AlignCenter);
    horizontalLayout.addWidget(&labelHost);

    btnRemove.setObjectName(QString::fromUtf8("btnRemove"));
    btnRemove.setMinimumSize(QSize(27, 27));
    btnRemove.setMaximumSize(QSize(27, 27));
    btnRemove.setIcon(QIcon(":/img/cross.png"));
    btnRemove.setFlat(true);
    horizontalLayout.addWidget(&btnRemove);

    horizontalLayout.setStretch(1, 2);
    horizontalLayout.setStretch(2, 1);

    connect(&btnRemove, &QPushButton::clicked, this, [this]{ emit removeClicked(uuid); });
}

DeviceItemWidget::~DeviceItemWidget()
{
}

void DeviceItemWidget::setUuid(const QUuid &_uuid)
{
    uuid = _uuid;
}

QUuid DeviceItemWidget::getUuid() const
{
    return uuid;
}

void DeviceItemWidget::setName(const QString &name)
{
    labelName.setText(name);
}

void DeviceItemWidget::setHost(const QHostAddress &host)
{
    labelHost.setText(QHostAddress(host.toIPv4Address()).toString());
}

void DeviceItemWidget::setState(SharedCursor::ConnectionState _state)
{
    state = _state;
    switch(state) {
    case SharedCursor::Unknown:
        labelStatus.setPixmap(pixmapDisconnected);
        break;
    case SharedCursor::Disconnected:
        labelStatus.setPixmap(pixmapDisconnected);
        break;
    case SharedCursor::Connected:
        labelStatus.setPixmap(pixmapConnected);
        break;
    case SharedCursor::Waiting:
        labelStatus.setPixmap(pixmapDisconnected);
        break;
    }
}

void DeviceItemWidget::setSelfState(bool self)
{
    selfState = self;
    btnRemove.setVisible(!self);

    if (self)
        labelStatus.setPixmap(pixmapConnected);
}
