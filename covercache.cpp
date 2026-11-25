// covercache.cpp
#include "covercache.h"
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QCryptographicHash>

namespace CoverCache {

    QString keyFor(const QString &artist, const QString &album, const QString &path)
    {
        QString material;
        if (!artist.isEmpty() && !album.isEmpty())
            material = artist + QChar(0x1f) + album;   // 0x1f = unit separator, won't appear in tags
        else
            material = path;                            // fallback: per-track key

        return QString::fromLatin1(
            QCryptographicHash::hash(material.toUtf8(),
                                     QCryptographicHash::Sha1).toHex());
    }

    QString filePathFor(const QString &key)
    {
        // CacheLocation is per-user, per-app, and cleared by the OS/user.
        // Use AppDataLocation instead if covers must survive cache cleaning.
        const QString dir = QStandardPaths::writableLocation(
                                QStandardPaths::CacheLocation)
                            + "/covers";
        QDir().mkpath(dir);            // idempotent
        return dir + "/" + key;
    }

    bool store(const QString &key, const QByteArray &bytes)
    {
        if (bytes.isEmpty())
            return false;

        const QString path = filePathFor(key);

        // QSaveFile writes to a temp file and renames on commit(), so a
        // crash mid-write can't leave a truncated cover behind. This matters
        // because the worker thread writes these concurrently.
        QSaveFile f(path);
        if (!f.open(QIODevice::WriteOnly))
            return false;
        if (f.write(bytes) != bytes.size())
            return false;
        return f.commit();
    }

    QByteArray load(const QString &key)
    {
        QFile f(filePathFor(key));
        if (!f.open(QIODevice::ReadOnly))
            return {};
        return f.readAll();
    }

} // namespace CoverCache