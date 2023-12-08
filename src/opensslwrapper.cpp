#include "opensslwrapper.h"

#include <QtEndian>
#include <QRandomGenerator>
#include <openssl/aes.h>

static const int AESKeySize = 256;
static const int LengthSize = 4;

OpenSslWrapper::OpenSslWrapper()
    : initVector(AES_BLOCK_SIZE, Qt::Uninitialized)
{
}

void OpenSslWrapper::setKey(const QByteArray &key)
{
    const int keySize = 32;
    QByteArray result("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x30\x31", keySize);
    std::copy(key.data(), key.data() + ((key.size() < keySize) ? key.size() : keySize), result.begin());
    AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(result.data()), AESKeySize, &eKey);
    AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(result.data()), AESKeySize, &dKey);
}

void OpenSslWrapper::encrypt(const QByteArray& input, QByteArray &output)
{
    quint32 size = input.size();
    int encryptionLength = size + LengthSize;
    if (encryptionLength % AES_BLOCK_SIZE != 0) {
        encryptionLength = ((encryptionLength / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
    }

    inputData.resize(size + LengthSize);
    qToBigEndian(size, inputData.data());
    memcpy(inputData.data() + LengthSize, input, size);
    memset(inputData.data() + size + LengthSize, 0, encryptionLength - (size + LengthSize));

    QRandomGenerator::global()->generate(initVector.begin(), initVector.end());

    output.resize(encryptionLength + AES_BLOCK_SIZE);
    memcpy(output.data(), initVector, AES_BLOCK_SIZE);

    AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(inputData.data()),
                    reinterpret_cast<unsigned char *>(output.data() + AES_BLOCK_SIZE),
                    inputData.size(), &eKey, reinterpret_cast<unsigned char *>(initVector.data()), AES_ENCRYPT);
}

bool OpenSslWrapper::decrypt(const QByteArray &input, QByteArray &output)
{
    return decrypt(input.data(), input.size(), output);
}

bool OpenSslWrapper::decrypt(const char *input, int _size, QByteArray &output)
{
    output.clear();

    if (_size < AES_BLOCK_SIZE) return false;
    if (_size % AES_BLOCK_SIZE != 0) return false;

    memcpy(initVector.data(), input, AES_BLOCK_SIZE);
    decrypted.resize(_size);

    AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(input + AES_BLOCK_SIZE),
                    reinterpret_cast<unsigned char *>(decrypted.data()),
                    _size - AES_BLOCK_SIZE, &dKey, reinterpret_cast<unsigned char *>(initVector.data()), AES_DECRYPT);

    quint32 size = qFromBigEndian<quint32>(decrypted.data());

    if (size > static_cast<quint32>(decrypted.size() + LengthSize))
        return false;

    output.resize(size);
    memcpy(output.data(), decrypted.data() + LengthSize, size);
    return true;
}
