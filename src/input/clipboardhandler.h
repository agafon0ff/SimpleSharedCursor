#pragma once

#include <QObject>
#include "utils.h"

class ClipboardHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardHandler(QObject *parent = nullptr);

public slots:
    void setCurrentUuid(const QUuid &uuid);
    void setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state);
    void setRemoteControlState(const QUuid &master, const QUuid &slave);

private slots:
    void onClipboardChanged();

signals:

private:
    SharedCursor::ControlState controlState = SharedCursor::SelfControl;
    QUuid ownUuid;
    QUuid controlledByUuid;
};
