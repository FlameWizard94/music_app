#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <QListWidget>
#include <QTreeWidgetItem>
#include "musicData.h"

class Playlist: public QListWidget
{
    Q_OBJECT
public:
    explicit Playlist(QWidget *parent = nullptr);
    void buildFrom(MusicData *musicData,
                   const QString &name,
                   const QString &album,
                   const QString &artist,
                   bool update = false);
    void appendSong(Song song);
    void appendSong(QTreeWidgetItem * item,
                    const QList<int> &roles = SongRoles::TrackRoles);
    void setName(QString name);
    QString getName();


private:
    void addSongData(QListWidgetItem * item, Song song);
    QString m_name;

};

#endif // PLAYLIST_H
