#include "musicData.h"
#include <QListView>
#include <QListWidget>
#include <QStringList>
#include <QDirIterator>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QSettings>
#include <QDebug>

MusicData::MusicData(QObject *parent)
    :QObject{parent}
{
}

void MusicData::buildFrom(const QString &dir, bool update)
{
    m_songs.clear();
    m_pendingPaths.clear();
    m_currentDir = dir;
    m_player = new QMediaPlayer(this);

    // If not forcing an update, try the cache first.
    if (!update && loadFromSettings(dir)) {
        emit finished();
        return;
    }

    QSettings settings;
    settings.setValue("music_dir", dir);

    static const QStringList filters = {
        "*.mp3", "*.wav", "*.flac", "*.m4a", "*.ogg", "*.opus"
    };

    QDirIterator it(dir, filters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        m_pendingPaths << it.next();

    if (m_pendingPaths.isEmpty()) {
        saveToSettings();      // write an empty list so we don't rescan every launch
        emit finished();
        return;
    }

    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &MusicData::onMediaStatusChanged);

    loadNext();
}


void MusicData::loadNext()
{

    if (m_pendingPaths.isEmpty())
    {
        saveToSettings();
        emit finished();
        return;
    }

    m_currentPath = m_pendingPaths.takeFirst();
    m_player->setSource(QUrl::fromLocalFile(m_currentPath));
}

void MusicData::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia)
    {
        extractData(m_currentPath);
        loadNext();
    }
}

int MusicData::songCount() { return m_songs.count(); }

Song MusicData::getSong(int index){ return m_songs.at(index); }

void MusicData::extractData(QString path){
    QMediaMetaData metaData = m_player->metaData();
    QString artist = metaData.value(QMediaMetaData::ContributingArtist).toString();
    QString album  = metaData.value(QMediaMetaData::AlbumTitle).toString();
    QString genre  = metaData.value(QMediaMetaData::Genre).toString();
    QString title  = metaData.value(QMediaMetaData::Title).toString();
    qlonglong durationMs = metaData.value(QMediaMetaData::Duration).toLongLong();

    Song song;
    song.id = m_songs.size();
    song.path = path;
    song.album = album;
    song.artist = artist;
    song.duration = durationMs;
    song.genre = genre;
    song.title = title;

    qDebug() << song.id;
    qDebug() << song.title;
    qDebug() << song.path;
    qDebug() << song.album;
    qDebug() << song.artist;
    qDebug() << song.duration;
    qDebug() << song.genre;

    m_songs.append(song);
}

void MusicData::saveToSettings() const
{
    QSettings settings;

    QVariantList list;
    list.reserve(m_songs.size());

    for (const Song &s : m_songs) {
        QVariantMap m;
        m["id"]         = s.id;
        m["path"]       = s.path;
        m["title"]      = s.title;
        m["artist"]     = s.artist;
        m["album"]      = s.album;
        m["genre"]      = s.genre;
        m["duration"]   = s.duration;
        list << m;
    }

    settings.setValue("songs/" + m_currentDir, list);
    settings.setValue("music_dir", m_currentDir);
    settings.sync();
}

bool MusicData::loadFromSettings(const QString &dir)
{
    QSettings settings;
    const QVariantList list = settings.value("songs/" + dir).toList();
    if (list.isEmpty())
        return false;

    m_songs.clear();
    m_songs.reserve(list.size());

    for (const QVariant &v : list) {
        const QVariantMap m = v.toMap();
        Song s;
        s.id        = m.value("id").toInt();
        s.path      = m.value("path").toString();
        s.title     = m.value("title").toString();
        s.artist    = m.value("artist").toString();
        s.album     = m.value("album").toString();
        s.genre     = m.value("genre").toString();
        s.duration  = m.value("duration").toLongLong();
        m_songs << s;
    }

    m_currentDir = dir;
    return true;
}