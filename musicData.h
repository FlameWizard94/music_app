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
    qint64  duration = 0,
            id = 0;
};

namespace SongRoles {
enum Role {
    Path        = Qt::UserRole,
    Duration    = Qt::UserRole + 1,
    Artist      = Qt::UserRole + 2,
    Album       = Qt::UserRole + 3,
    Title       = Qt::UserRole + 4,
    Genre       = Qt::UserRole + 5,
    Id          = Qt::UserRole + 6,
};

inline const QList<int> TrackRoles = {
    Path, Duration, Artist, Album, Title, Genre, Id
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
    const Song &getSong(int index) const;
    int songCount() const;


signals:
    void finished();

private:
    QVector<Song> m_songs;
    QVector<QString> m_paths;
    QFutureWatcher<QVector<Song>> m_watcher;
    QString m_currentDir;
    void saveToSettings() const;
    bool loadFromSettings(const QString &dir);
    Song extractDataFuture(int index) const;
};




#endif // MUSIC_DATA_H
