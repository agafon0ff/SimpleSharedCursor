#include "opensslwrapper.h"

#include <openssl/evp.h>
#include <memory>

static const int BLOCK_SIZE = 16;
static const int EVP_KEY_SIZE = 32;

void OpenSslWrapper::setKey(const QByteArray &key)
{
    _key.resize(EVP_KEY_SIZE);
    std::iota(_key.begin(), _key.end(), 0x00);
    std::copy(key.data(), key.data() + ((key.size() < EVP_KEY_SIZE) ? key.size() : EVP_KEY_SIZE), _key.begin());

    _iv = "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x30\x31\x32\x33\x34\x35";
    std::copy(key.data(), key.data() + ((key.size() < _iv.size()) ? key.size() : _iv.size()), _iv.begin());
}

bool OpenSslWrapper::encrypt(const char *input, int size, QByteArray &output)
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);

    int rc = EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL,
                                reinterpret_cast<const unsigned char*>(_key.constData()),
                                reinterpret_cast<const unsigned char*>(_iv.constData()));
    if (rc != 1) return false;

    output.resize(size + BLOCK_SIZE);
    int out_len1 = output.size();

    rc = EVP_EncryptUpdate(ctx.get(), reinterpret_cast<unsigned char*>(output.data()), &out_len1,
                           reinterpret_cast<const unsigned char*>(input), size);
    if (rc != 1) return false;

    int out_len2 = output.size() - out_len1;
    rc = EVP_EncryptFinal_ex(ctx.get(), reinterpret_cast<unsigned char*>(output.data()) + out_len1, &out_len2);
    if (rc != 1) return false;

    output.resize(out_len1 + out_len2);
    return true;
}

bool OpenSslWrapper::decrypt(const char *input, int size, QByteArray &output)
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);

    int rc = EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL,
                            reinterpret_cast<const unsigned char*>(_key.constData()),
                            reinterpret_cast<const unsigned char*>(_iv.constData()));
    if (rc != 1) return false;

    output.resize(size);
    int out_len1 = output.size();

    rc = EVP_DecryptUpdate(ctx.get(), reinterpret_cast<unsigned char*>(output.data()), &out_len1,
                           reinterpret_cast<const unsigned char*>(input), size);
    if (rc != 1) return false;

    int out_len2 = output.size() - out_len1;
    rc = EVP_DecryptFinal_ex(ctx.get(), reinterpret_cast<unsigned char*>(output.data()) + out_len1, &out_len2);
    if (rc != 1) return false;

    output.resize(out_len1 + out_len2);
    return true;
}
