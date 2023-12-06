#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDebug>

#include "clipboardhandler.h"

ClipboardHandler::ClipboardHandler(QObject *parent)
    : QObject{parent}
{
    const QClipboard *clipboard = QApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardHandler::onClipboardChanged);
}

void ClipboardHandler::setCurrentUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;
    ownUuid = uuid;
    controlledByUuid = ownUuid;
}

void ClipboardHandler::setConnectionState(const QUuid &uuid, SharedCursor::ConnectionState state)
{
    qDebug() << Q_FUNC_INFO << uuid << state;

    if (uuid == controlledByUuid && state != SharedCursor::ConnectionState::Connected) {
        controlState = SharedCursor::SelfControl;
        controlledByUuid = ownUuid;
    }
}

void ClipboardHandler::setRemoteControlState(const QUuid &master, const QUuid &slave)
{
    controlledByUuid = master;
    controlState = slave == ownUuid ? SharedCursor::SelfControl : SharedCursor::Slave;

    qDebug() << Q_FUNC_INFO << master << slave << controlState;
}

void ClipboardHandler::onClipboardChanged()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    qDebug() << Q_FUNC_INFO << clipboard->text();
    qDebug() << Q_FUNC_INFO
             << mimeData->hasImage()
             << mimeData->hasUrls()
             << mimeData->hasHtml()
             << mimeData->hasText();
}
