#include "playlist.h"
#include "musicData.h"
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

void Playlist::buildFrom(MusicData *musicData,
                         const QString &name,
                         const QString &album,
                         const QString &artist,
                         bool update)
{
    clear();  //remove all items from listWidget

    const QString key = "pl_" + name;
    m_name = name;

    QSettings settings;
    if (!update) {
        QStringList saved = settings.value(key).toStringList();
        if (!saved.isEmpty()) {
            addItems(saved);
            return;
        }
    }

    if (!musicData)
        return;

    QStringList playlist_data;

    int num = musicData->songCount();
    for (int i = 0; i < num; ++i) {
        Song song = musicData->getSong(i);

        const QString itemAlbum  = song.album;
        const QString itemArtist = song.artist;

        if (!album.isEmpty()  && album.compare(itemAlbum,  Qt::CaseInsensitive) != 0)
            continue;
        if (!artist.isEmpty() && artist.compare(itemArtist, Qt::CaseInsensitive) != 0)
            continue;

        QListWidgetItem *item = new QListWidgetItem;
        addSongData(item, song);
        addItem(item);
        playlist_data << item->text();
    }

    settings.setValue(key, playlist_data);
}

void Playlist::setName(QString name)
{
    QSettings settings;
    m_name = name;
    QStringList playlist_data;

    for(int i = 0; i < count(); i++)
    {
        playlist_data << item(i)->text();
    }
    settings.setValue(m_name, playlist_data);
}

QString Playlist::getName(){ return m_name; }

void Playlist::appendSong(Song song)
{
    QListWidgetItem *item = new QListWidgetItem;
    addSongData(item, song);
    addItem(item);
}


void Playlist::appendSong(QTreeWidgetItem * treeItem,
                          const QList<int> &roles)
{
    QListWidgetItem *new_item = new QListWidgetItem;
    for (int role : roles)
        new_item->setData(role, treeItem->data(0, role).toString());

    new_item->setText(treeItem->text(0));
    addItem(new_item);
}

void Playlist::addSongData(QListWidgetItem * item, Song song)
{
    item->setText(song.title);
    item->setData(SongRoles::Path, song.path);
    item->setData(SongRoles::Artist, song.artist);
    item->setData(SongRoles::Album, song.album);
    item->setData(SongRoles::Duration, song.duration);
    item->setData(SongRoles::Genre, song.genre);
    item->setData(SongRoles::Title, song.title);
    return;
}