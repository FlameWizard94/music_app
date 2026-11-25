#ifndef COVERCACHE_H
#define COVERCACHE_H

// covercache.h
#pragma once
#include <QByteArray>
#include <QString>

namespace CoverCache {

    // Stable key for an album's cover. Hash artist+album so every track in
    // an album maps to the same cache file. Fall back to the track path if
    // either field is empty, so compilation tracks with missing tags don't
    // all collide on the empty key.
    QString keyFor(const QString &artist, const QString &album, const QString &path);

    // Absolute path of the cache file for a key.
    QString filePathFor(const QString &key);

    // Write bytes atomically (temp file + rename). Returns false on failure.
    bool store(const QString &key, const QByteArray &bytes);

    // Read bytes back. Returns empty QByteArray if absent.
    QByteArray load(const QString &key);

} // namespace CoverCache

#endif // COVERCACHE_H
