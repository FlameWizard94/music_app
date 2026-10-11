#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <QListWidget>
#include "music_data.h"

class Playlist: public QListWidget
{
    Q_OBJECT
public:
    explicit Playlist(QWidget *parent = nullptr);

    void buildFrom(MusicData *music_data,
                   const QString &name,
                   const QString &album,
                   const QString &artist,
                   bool update = false);



};

#endif // PLAYLIST_H
