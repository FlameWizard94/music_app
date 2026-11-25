#ifndef MUSIC_DATA_H
#define MUSIC_DATA_H

#include <QListView>
#include <QListWidget>
#include <QString>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QVector>
#include <QThreadPool>
#include <QMutex>
#include <QFutureWatcher>
#include <qdatetime.h>

struct Song {
    QString path, title, album, artist, genre;
    QString coverKey;
    qint64  duration = 0,
        track = 0,
        release;
    //qitn64 id = 0;
    //QImage coverArt;
    //QByteArray coverArtBytes;
    QDate created;
    //QSharedPointer<AudioSample> waveformData;
};

enum class SortMode { TrackNumber, Created };

inline const QList<QPair<SortMode, QString>> kSortModes =
{
    {SortMode::TrackNumber,     "Track"},
    {SortMode::Created,         "Date Created"},
};

namespace SongRoles {
enum Role {
    Path            = Qt::UserRole,
    Duration        = Qt::UserRole + 1,
    Artist          = Qt::UserRole + 2,
    Album           = Qt::UserRole + 3,
    Title           = Qt::UserRole + 4,
    Genre           = Qt::UserRole + 5,
    TrackNumber     = Qt::UserRole + 6,
    Created         = Qt::UserRole + 7,
    Release         = Qt::UserRole + 8,
};

inline const QList<int> TrackRoles = {
    Path, Duration, Artist, Album, Title, Genre, TrackNumber
    //, Id
};
}

enum class GroupMode { AlbumOnly, AlbumArtist, AlbumGenre, ArtistAlbumGenre };

inline const QList<QPair<GroupMode, QString>> kGroupModes =
{
    { GroupMode::AlbumOnly,          "Album"                },
    { GroupMode::AlbumArtist,        "Album / Artist"       },
    { GroupMode::AlbumGenre,         "Album / Genre"        },
    { GroupMode::ArtistAlbumGenre,   "Album / Artist / Genre" },
};




class MusicData : public QObject
{
    Q_OBJECT
public:
    explicit MusicData(QObject *parent = nullptr);
    void buildFrom(const QString &dir,
                   bool update = false);
    const Song * getSong(const QString path) const;
    int songCount() const;
    const QHash<QString, Song> * allSongs() const;
    QImage getCover(QString path) const;
    QByteArray getCoverBytes(QString path) const;
    //static QByteArray imageToBytes(const QImage &image);
    //static QImage bytesToImage(const QByteArray &bytes);


signals:
    void scanProgress(int done, int total);
    void finished();

private:
    //QVector<Song> m_songs;
    QHash<QString, Song> m_songs;
    QVector<QString> m_paths;
    QFutureWatcher<QHash<QString, Song>> m_watcher;
    //QFutureWatcher<QVector<Song>> m_watcher;
    QString m_currentDir;
    void saveToSettings() const;
    bool loadFromSettings(const QString &dir);
    Song extractDataFuture(QString path) const;
};




#endif // MUSIC_DATA_H
