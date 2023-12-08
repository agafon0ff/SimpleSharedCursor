#pragma once

#include <QString>
#include <QObject>

#include <openssl/aes.h>

class OpenSslWrapper
{
public:
    OpenSslWrapper();

    void setKey(const QByteArray& key);
    void encrypt(const QByteArray &input, QByteArray &output);
    bool decrypt(const QByteArray& input, QByteArray &output);
    bool decrypt(const char* input, int size, QByteArray &output);

private:
    AES_KEY eKey, dKey;
    QByteArray initVector, inputData, decrypted;
};

