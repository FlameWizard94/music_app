#ifndef MUSIC_DATA_H
#define MUSIC_DATA_H

#include <QListView>
#include <QListWidget>
#include <QString>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QVector>

struct Song {
    QString path, title, album, artist, genre;
    qint64  duration = 0,
            id = 0;
};

class MusicData : public QObject
{
    Q_OBJECT
public:
    explicit MusicData(QObject *parent = nullptr);
    void buildFrom(const QString &dir,
                   bool update = false);
    Song getSong(int index);
    int songCount();


signals:
    void finished();

private:
    //QMediaMetaData * metaData;
    QVector<Song> m_songs;
    QMediaPlayer * m_player;
    QStringList m_pendingPaths;
    QString m_currentPath;
    QString m_currentDir;
    void saveToSettings() const;
    bool loadFromSettings(const QString &dir);
    void extractData(QString path);
    void loadNext();

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
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


#endif // MUSIC_DATA_H
