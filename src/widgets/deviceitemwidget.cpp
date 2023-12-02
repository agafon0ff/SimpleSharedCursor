#include "deviceitemwidget.h"
#include "ui_deviceitemwidget.h"

DeviceItemWidget::DeviceItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceItemWidget)
{
    ui->setupUi(this);

    connect(ui->btnRemove, &QPushButton::clicked, this, [this]{ emit removeClicked(uuid); });
}

DeviceItemWidget::~DeviceItemWidget()
{
    delete ui;
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
    ui->labelName->setText(name);
}

void DeviceItemWidget::setHost(const QHostAddress &host)
{
    ui->labelHost->setText(QHostAddress(host.toIPv4Address()).toString());
}

void DeviceItemWidget::setState(SharedCursor::ConnectionState _state)
{
    state = _state;
    switch(state) {
    case SharedCursor::Unknown:
        ui->labelStatus->setText("?");
        break;
    case SharedCursor::Disconnected:
        ui->labelStatus->setText("-");
        break;
    case SharedCursor::Connected:
        ui->labelStatus->setText("+");
        break;
    case SharedCursor::Waiting:
        ui->labelStatus->setText(".");
        break;
    }
}
