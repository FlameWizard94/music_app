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

struct Song {
    QString path, title, album, artist, genre;
    qint64  duration = 0;
    //qitn64 id = 0;
    //QSharedPointer<QImage> albumArt;
    //QSharedPointer<AudioSample> waveformData;

};

namespace SongRoles {
enum Role {
    Path        = Qt::UserRole,
    Duration    = Qt::UserRole + 1,
    Artist      = Qt::UserRole + 2,
    Album       = Qt::UserRole + 3,
    Title       = Qt::UserRole + 4,
    Genre       = Qt::UserRole + 5,
    //Id          = Qt::UserRole + 6,
};

inline const QList<int> TrackRoles = {
    Path, Duration, Artist, Album, Title, Genre
    //, Id
};
}

enum class SortMode { AlbumOnly, AlbumArtist, AlbumGenre, ArtistAlbumGenre };

inline const QList<QPair<SortMode, QString>> kSortModes = {
    { SortMode::AlbumOnly,          "Album"                },
    { SortMode::AlbumArtist,        "Album / Artist"       },
    { SortMode::AlbumGenre,         "Album / Genre"        },
    { SortMode::ArtistAlbumGenre,   "Album / Artist / Genre" },
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
