#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <QByteArray>
#include <QString>

class CryptoUtils
{
public:
    static QByteArray xorEncryptDecrypt(const QByteArray& data, const QString& key);
    static QString hashPassword(const QString& password);

private:
    CryptoUtils() = delete;
};

#endif // CRYPTOUTILS_H