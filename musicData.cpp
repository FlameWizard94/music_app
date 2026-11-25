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
#include <QBuffer>
#include <QByteArray>

#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>

#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>

#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/mp4item.h>

#include <taglib/flacfile.h>

#include <QtConcurrentMap>

#include <covercache.h>

MusicData::MusicData(QObject *parent)
    :QObject{parent}
{
    connect(&m_watcher, &QFutureWatcher<QVector<Song>>::finished, this, [this]()
    {
        m_songs = m_watcher.result();
        saveToSettings();
        emit finished();
    });
    // MusicData constructor
    connect(&m_watcher, &QFutureWatcher<QVector<Song>>::progressRangeChanged,
        this, [this](int min, int max)
    {
        emit scanProgress(min, max == min ? 0 : m_paths.size());   // range known
    });
    connect(&m_watcher, &QFutureWatcher<QVector<Song>>::progressValueChanged,
        this, [this](int v)
    {
        emit scanProgress(v, m_watcher.progressMaximum());
    });
}

void MusicData::buildFrom(const QString &dir, bool update)
{
    m_currentDir = dir;
    m_songs.clear();
    m_paths.clear();

    if (!update &&
        loadFromSettings(dir)) {
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
        m_paths.append(it.next());

    if (m_paths.isEmpty()) {
        saveToSettings();
        emit finished();
        return;
    }


    QFuture<QHash<QString, Song>> future = QtConcurrent::mappedReduced(
        m_paths,
        [this](QString path) { return extractDataFuture(path); },
        [](QHash<QString, Song> &acc, const Song &s) { acc[s.path] = s; });

    m_watcher.setFuture(future);
}

Song MusicData::extractDataFuture(QString path) const
{
    Song s;
    s.path = path;
    QFileInfo fileInfo(path);
    QDateTime birthTime = fileInfo.birthTime();

    if (birthTime.isValid())
    {
        s.created = birthTime.date();
    }
    else{
        s.created = birthTime.date();
    }


    TagLib::FileRef ref(QFile::encodeName(s.path).constData());
    if (ref.isNull() || !ref.tag())
        return s;

    const auto *tag = ref.tag();
    s.title  = QString::fromStdWString(tag->title().toWString());
    if (s.title.isEmpty())
    {
        s.title = fileInfo.completeBaseName();
    }
    s.artist = QString::fromStdWString(tag->artist().toWString());
    s.album  = QString::fromStdWString(tag->album().toWString());
    s.genre  = QString::fromStdWString(tag->genre().toWString());
    s.duration = ref.audioProperties()
                     ? ref.audioProperties()->lengthInMilliseconds()
                     : 0;
    s.track = tag->track();
    s.release = tag->year();

    s.coverKey = CoverCache::keyFor(s.artist, s.album, s.path);
    if (!QFile::exists(CoverCache::filePathFor(s.coverKey))) {
        // Only extract (and write) if it's not already cached. Saves
        // repeated TagLib picture reads when many tracks share an album.
        const QByteArray bytes = getCoverBytes(s.path);
        if (!bytes.isEmpty())
            CoverCache::store(s.coverKey, bytes);
    }

    return s;
}

QImage MusicData::getCover(QString path) const
{
    QImage coverArt;
    QByteArray coverArtBytes;
    if (path.endsWith(".mp3"))
    {
        TagLib::MPEG::File file(path.toUtf8().constData());
        TagLib::ID3v2::Tag *tag = file.ID3v2Tag();
        if (tag)
        {
            TagLib::ID3v2::FrameList frames = tag->frameList("APIC");
            if (!frames.isEmpty())
            {
                auto *frame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
                coverArtBytes = frame->picture().data();
                coverArt.loadFromData((const uchar *)frame->picture().data(), frame->picture().size());
                return coverArt;
            }
        }
    }
    else if (path.endsWith(".flac"))
    {
        TagLib::FLAC::File file(path.toUtf8().constData());
        const TagLib::List<TagLib::FLAC::Picture*>& picList = file.pictureList();
        if (!picList.isEmpty())
        {
            TagLib::FLAC::Picture* pic = picList[0];
            coverArtBytes = pic->data().data();
            coverArt.loadFromData((const uchar *)pic->data().data(), pic->data().size());
            return coverArt;
        }
    }
    else if (path.endsWith(".m4a") || path.endsWith(".mp4"))
    {
        TagLib::MP4::File file(path.toUtf8().constData());
        TagLib::MP4::Tag* tag = file.tag();
        if (tag)
        {
            TagLib::MP4::ItemMap itemsMap = tag->itemMap();
            TagLib::MP4::Item coverItem = itemsMap["covr"];
            TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
            if (!coverArtList.isEmpty())
            {
                TagLib::MP4::CoverArt mCoverArt = coverArtList.front();
                coverArtBytes = mCoverArt.data().data();
                coverArt.loadFromData((const uchar *) mCoverArt.data().data(), mCoverArt.data().size());
                return coverArt;
            }
        }
    }
    return coverArt;
}

QByteArray MusicData::getCoverBytes(QString path) const
{
    QByteArray coverArtBytes;
    if (path.endsWith(".mp3"))
    {
        TagLib::MPEG::File file(path.toUtf8().constData());
        TagLib::ID3v2::Tag *tag = file.ID3v2Tag();
        if (tag)
        {
            TagLib::ID3v2::FrameList frames = tag->frameList("APIC");
            if (!frames.isEmpty())
            {
                auto *frame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
                const TagLib::ByteVector &bv = frame->picture();
                coverArtBytes = QByteArray(bv.data(), static_cast<int>(bv.size()));
                return coverArtBytes;
            }
        }
    }
    else if (path.endsWith(".flac"))
    {
        TagLib::FLAC::File file(path.toUtf8().constData());
        const TagLib::List<TagLib::FLAC::Picture*>& picList = file.pictureList();
        if (!picList.isEmpty())
        {
            TagLib::FLAC::Picture* pic = picList[0];
            const TagLib::ByteVector &bv = pic->data();
            coverArtBytes = QByteArray(bv.data(), static_cast<int>(bv.size()));
            return coverArtBytes;
        }
    }
    else if (path.endsWith(".m4a") || path.endsWith(".mp4"))
    {
        TagLib::MP4::File file(path.toUtf8().constData());
        TagLib::MP4::Tag* tag = file.tag();
        if (tag)
        {
            TagLib::MP4::ItemMap itemsMap = tag->itemMap();
            TagLib::MP4::Item coverItem = itemsMap["covr"];
            TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
            if (!coverArtList.isEmpty())
            {
                TagLib::MP4::CoverArt mCoverArt = coverArtList.front();
                const TagLib::ByteVector &bv = mCoverArt.data();
                coverArtBytes = QByteArray(bv.data(), static_cast<int>(bv.size()));
                return coverArtBytes;
            }
        }
    }
    return coverArtBytes;
}


int MusicData::songCount() const { return m_songs.count(); }

const Song * MusicData::getSong(const QString path) const
{
    auto it = m_songs.find(path);

    if (it != m_songs.end()) {
        return &it.value();
    } else {
        return nullptr;
    }
}

const QHash<QString, Song> * MusicData::allSongs() const{ return &m_songs; };


void MusicData::saveToSettings() const
{
    QSettings settings;

    QVariantList list;
    list.reserve(m_songs.size());

    for (const auto& s : m_songs) {
        QVariantMap m;
        //m["id"]         = s.id;
        m["path"]       = s.path;
        m["title"]      = s.title;
        m["artist"]     = s.artist;
        m["album"]      = s.album;
        m["genre"]      = s.genre;
        m["duration"]   = s.duration;
        m["track"]      = s.track;
        m["release"]    = s.release;
        m["created"]    = s.created;
        m["coverKey"] = s.coverKey;
        //m["coverArt"]   = s.coverArtBytes;
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
        //s.id        = m.value("id").toInt();
        s.path      = m.value("path").toString();
        s.title     = m.value("title").toString();
        s.artist    = m.value("artist").toString();
        s.album     = m.value("album").toString();
        s.genre     = m.value("genre").toString();
        s.duration  = m.value("duration").toLongLong();
        s.track     = m.value("track").toInt();
        s.created   = m.value("created").toDate();
        s.release   = m.value("release").toInt();
        s.coverKey = m.value("coverKey").toString();
        //s.coverArtBytes  = m.value("coverArt").toByteArray();
        m_songs[s.path] = s;
    }

    m_currentDir = dir;
    return true;
}

