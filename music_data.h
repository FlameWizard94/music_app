#ifndef MUSIC_DATA_H
#define MUSIC_DATA_H

#include <QListView>
#include <QListWidget>
#include <QString>
#include <QMediaMetaData>
#include <QMediaPlayer>

class MusicData : public QListWidget
{
    Q_OBJECT
public:
    explicit MusicData(QWidget *parent = nullptr);
    void buildFrom(const QString &dir,
                   bool update = false);
signals:
    void finished();
private:
    //QMediaMetaData * metaData;
    QMediaPlayer * player;
    QStringList m_pendingPaths;
    QStringList new_data;
    QString m_currentPath;
    QString current_dir;
private slots:
    void extractData(QString path);
    void loadNext();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
};

namespace SongRoles {
enum Role {
    Path     = Qt::UserRole,
    Duration = Qt::UserRole + 1,
    Artist   = Qt::UserRole + 2,
    Album    = Qt::UserRole + 3,
    Title    = Qt::UserRole + 4,
    Genre    = Qt::UserRole + 5,
    };
}

#endif // MUSIC_DATA_H
