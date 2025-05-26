#include "deviceitemwidget.h"

#include <QPainter>

DeviceItemWidget::DeviceItemWidget(QWidget *parent) :
    QWidget(parent),
    _pixmapConnected{"://img/connected.png"},
    _pixmapDisconnected{"://img/disconnected.png"}
{
    setMinimumHeight(30);
    setMaximumHeight(30);

    setLayout(&_horizontalLayout);
    _horizontalLayout.setSpacing(0);
    _horizontalLayout.setContentsMargins(0, 0, 0, 0);

    _labelStatus.setMinimumSize(QSize(30, 30));
    _labelStatus.setMaximumSize(QSize(30, 30));
    _labelStatus.setPixmap(QPixmap(":/img/disconnected.png"));
    _labelStatus.setScaledContents(true);
    _labelStatus.setAlignment(Qt::AlignCenter);
    _labelStatus.setMargin(3);
    _horizontalLayout.addWidget(&_labelStatus);

    _labelName.setObjectName(QString::fromUtf8("labelName"));
    _labelName.setAlignment(Qt::AlignCenter);
    _horizontalLayout.addWidget(&_labelName);

    _labelHost.setObjectName(QString::fromUtf8("labelHost"));
    _labelHost.setAlignment(Qt::AlignCenter);
    _horizontalLayout.addWidget(&_labelHost);

    _btnRemove.setObjectName(QString::fromUtf8("btnRemove"));
    _btnRemove.setMinimumSize(QSize(27, 27));
    _btnRemove.setMaximumSize(QSize(27, 27));
    _btnRemove.setIcon(QIcon(":/img/cross.png"));
    _btnRemove.setFlat(true);
    _horizontalLayout.addWidget(&_btnRemove);

    _horizontalLayout.setStretch(1, 2);
    _horizontalLayout.setStretch(2, 1);

    connect(&_btnRemove, &QPushButton::clicked, this, [this]{ emit removeClicked(_uuid); });
}

DeviceItemWidget::~DeviceItemWidget()
{
}

void DeviceItemWidget::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}

QUuid DeviceItemWidget::getUuid() const
{
    return _uuid;
}

void DeviceItemWidget::setName(const QString &name)
{
    _labelName.setText(name);
}

void DeviceItemWidget::setHost(const QHostAddress &host)
{
    _labelHost.setText(QHostAddress(host.toIPv4Address()).toString());
}

void DeviceItemWidget::setState(SharedCursor::ConnectionState _state)
{
    _state = _state;
    switch(_state) {
    case SharedCursor::Unknown:
        _labelStatus.setPixmap(_pixmapDisconnected);
        break;
    case SharedCursor::Disconnected:
        _labelStatus.setPixmap(_pixmapDisconnected);
        break;
    case SharedCursor::Connected:
        _labelStatus.setPixmap(_pixmapConnected);
        break;
    case SharedCursor::Waiting:
        _labelStatus.setPixmap(_pixmapDisconnected);
        break;
    }
}

void DeviceItemWidget::setSelfState(bool self)
{
    _selfState = self;
    _btnRemove.setVisible(!self);

    if (self)
        _labelStatus.setPixmap(_pixmapConnected);
}

void DeviceItemWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(QPen(QBrush(QColor(40, 40, 40)), 1));
    p.drawLine(0, height()-1, width()-1, height()-1);
}
