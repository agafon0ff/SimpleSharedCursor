#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDebug>

#include "clipboardhandler.h"
#include "global.h"

ClipboardHandler::ClipboardHandler(QObject *parent)
    : QObject{parent}
{
    const QClipboard *clipboard = QApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardHandler::onClipboardChanged);
    jsonMessage[SharedCursor::KEY_TYPE] = SharedCursor::KEY_CLIPBOARD;
}

void ClipboardHandler::setCurrentUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;
    uuidOwn = uuid;
    uuidMaster = uuidOwn;
}

void ClipboardHandler::setRemoteControlState(const QUuid &master, const QUuid &slave)
{
    if (slave != uuidOwn && !clipboardHolders.contains(slave)) {
        clipboardHolders.insert(slave, false);
    }

    if (uuidSlave != slave && uuidSlave == uuidOwn) {
        sendClipboard(slave);
    }

    uuidMaster = master;
    uuidSlave = slave;

    qDebug() << Q_FUNC_INFO << master << slave;
}

void ClipboardHandler::setClipboard(const QUuid &uuid, const QJsonObject &json)
{
    applyingClipboard = true;

    auto it = clipboardHolders.find(uuid);
    if (it != clipboardHolders.end()) {
        it.value() = true;
    }

    const QString &text = json.value(SharedCursor::KEY_VALUE).toString();
    QApplication::clipboard()->setText(text);

    applyingClipboard = false;
}

void ClipboardHandler::onClipboardChanged()
{
    if (applyingClipboard) {
        return;
    }

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasImage() || mimeData->hasUrls()) {
        // currently only text is available
        return;
    }

    for (auto it = clipboardHolders.begin(); it != clipboardHolders.end(); ++it) {
        it.value() = false;
    }
}

void ClipboardHandler::sendClipboard(const QUuid &uuid)
{
    auto it = clipboardHolders.find(uuid);
    if (it != clipboardHolders.end() && !it.value()) {
        jsonMessage[SharedCursor::KEY_VALUE] = QApplication::clipboard()->mimeData()->text();
        emit message(uuid, jsonMessage);
        it.value() = true;
    }
}
