#pragma once

#include <QHostAddress>
#include <QSharedPointer>
#include <QString>
#include <QLine>
#include <QUuid>
#include <QRect>

namespace SharedCursor
{
    inline const char* CONFIG_PATH = "config.json";
    inline const char* GEOMETRY_PATH = "geometry.json";
    inline const char* PROGRAM_NAME = "SimpleSharedCursor";

    inline const char* KEY_SEARCH_REQUEST = "searchRequest";
    inline const char* KEY_SEARCH_RESPONSE = "searchResponse";
    inline const char* KEY_CONNECT_REQUEST = "connectRequest";
    inline const char* KEY_CONNECT_RESPONSE = "connectResponse";
    inline const char* KEY_GEOMETRY = "geometry";
    inline const char* KEY_SPLITTER = "splitter";
    inline const char* KEY_KEYWORD = "keyword";
    inline const char* KEY_PORT_UDP = "portUdp";
    inline const char* KEY_PORT_TCP = "portTcp";
    inline const char* KEY_NAME = "name";
    inline const char* KEY_UUID = "uuid";
    inline const char* KEY_ALL = "all";
    inline const char* KEY_DEVICES = "devices";
    inline const char* KEY_HASHSUM = "hashsum";
    inline const char* KEY_NONCE = "nonce";
    inline const char* KEY_TYPE = "type";
    inline const char* KEY_VALUE = "value";
    inline const char* KEY_HOST = "host";
    inline const char* KEY_SELF = "self";
    inline const char* KEY_STATE = "state";
    inline const char* KEY_APPROVED = "approved";
    inline const char* KEY_SCREENS = "screens";
    inline const char* KEY_POSITION = "position";
    inline const char* KEY_TRANSITS = "transits";
    inline const char* KEY_CURSOR_DELTA = "delta";
    inline const char* KEY_CURSOR_POS = "cursorPos";
    inline const char* KEY_REMOTE_CONTROL = "remoteControl";
    inline const char* KEY_INPUT = "input";
    inline const char* KEY_MOUSE = "mouse";
    inline const char* KEY_KEYBOARD = "keyboard";
    inline const char* KEY_WHEEL = "wheel";
    inline const char* KEY_PRESSED = "pressed";
    inline const char* KEY_MASTER = "master";
    inline const char* KEY_SLAVE = "slave";

    inline const quint16 DEFAULT_TCP_PORT = 25786;
    inline const quint16 DEFAULT_UDP_PORT = 25787;
    inline const quint16 CONNECT_INTERVAL = 10000;

    enum ConnectionState {
        Unknown = 0,
        Disconnected,
        Connected,
        Waiting
    };

    enum ControlState {
        SelfControl = 0,
        Master,
        Slave
    };

    struct Transit
    {
        QLine line;
        QPoint pos;
        QUuid uuid;
    };

    struct Device
    {
        QUuid uuid;
        QString name;
        QHostAddress host;
        bool self = false;
        QPoint position;
        QVector<QRect> screens;
        QVector<Transit> transits;

        ConnectionState state = Disconnected;
    };

    inline bool operator==(const Device &d1, const Device &d2) { return d1.uuid == d2.uuid; }
};
