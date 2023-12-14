#pragma once

#include <QObject>

class OpenSslWrapper
{
public:
    void setKey(const QByteArray& key);
    bool encrypt(const char* input, int size, QByteArray &output);
    bool decrypt(const char* input, int size, QByteArray &output);

private:
    QByteArray key, iv;
};

