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
    _jsonMessage[SharedCursor::KEY_TYPE] = SharedCursor::KEY_CLIPBOARD;
}

void ClipboardHandler::setCurrentUuid(const QUuid &uuid)
{
    qDebug() << Q_FUNC_INFO << uuid;
    _uuidOwn = uuid;
    _uuidMaster = _uuidOwn;
}

void ClipboardHandler::setRemoteControlState(const QUuid &master, const QUuid &slave)
{
    if (slave != _uuidOwn && !_clipboardHolders.contains(slave)) {
        _clipboardHolders.insert(slave, false);
    }

    if (_uuidSlave != slave && _uuidSlave == _uuidOwn) {
        sendClipboard(slave);
    }

    _uuidMaster = master;
    _uuidSlave = slave;

    qDebug() << Q_FUNC_INFO << master << slave;
}

void ClipboardHandler::setClipboard(const QUuid &uuid, const QJsonObject &json)
{
    _applyingClipboard = true;

    auto it = _clipboardHolders.find(uuid);
    if (it != _clipboardHolders.end()) {
        it.value() = true;
    }

    const QString &text = json.value(SharedCursor::KEY_VALUE).toString();
    QApplication::clipboard()->setText(text);

    _applyingClipboard = false;
}

void ClipboardHandler::onClipboardChanged()
{
    if (_applyingClipboard) {
        return;
    }

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasImage() || mimeData->hasUrls()) {
        // currently only text is available
        return;
    }

    for (auto it = _clipboardHolders.begin(); it != _clipboardHolders.end(); ++it) {
        it.value() = false;
    }
}

void ClipboardHandler::sendClipboard(const QUuid &uuid)
{
    auto it = _clipboardHolders.find(uuid);
    if (it != _clipboardHolders.end() && !it.value()) {
        _jsonMessage[SharedCursor::KEY_VALUE] = QApplication::clipboard()->mimeData()->text();
        emit message(uuid, _jsonMessage);
        it.value() = true;
    }
}
