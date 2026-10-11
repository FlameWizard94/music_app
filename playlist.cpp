#include "playlist.h"
#include "music_data.h"
#include <QSettings>

//Get music dir and data from qsettings
//If playlist is saved in settings and update is false, get playlist data from settings
//Otherwise, make named playlist (listwidget) based on parameters
//Save playlist in settings
//playlists will have a naming scheme of "pl_{name}" to avoid potential future conflicts

//Replace album and artist with listWidget called parameters

Playlist::Playlist(QWidget *parent)
    : QListWidget{parent}
{
}

void Playlist::buildFrom(MusicData *music_data,
                         const QString &name,
                         const QString &album,
                         const QString &artist,
                         bool update)
{
    clear();  //remove all items from listWidget

    const QString key = "pl_" + name;

    QSettings settings;
    if (!update) {
        QStringList saved = settings.value(key).toStringList();
        if (!saved.isEmpty()) {
            addItems(saved);
            return;
        }
    }

    if (!music_data)
        return;

    QStringList playlist_data;
    int num = music_data->count();
    for (int i = 0; i < num; ++i) {
        QListWidgetItem *src = music_data->item(i);

        const QString itemAlbum  = src->data(SongRoles::Album).toString();
        const QString itemArtist = src->data(SongRoles::Artist).toString();

        if (!album.isEmpty()  && album.compare(itemAlbum,  Qt::CaseInsensitive) != 0)
            continue;
        if (!artist.isEmpty() && artist.compare(itemArtist, Qt::CaseInsensitive) != 0)
            continue;

        // Copy the item — do NOT addItem(src), it's owned by MusicData
        addItem(new QListWidgetItem(*src));
        playlist_data << src->text();
    }

    settings.setValue(key, playlist_data);
}
