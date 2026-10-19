#include <QTreeWidget>
#include "musicData.h"
#include "musicDataView.h"
#include <QMenu>

//Using hashtable instead of treewidget.finditems() for better time complexity
//O(N) vs o(N*M), N = num tracks, M = num unique keys/parent nodes


MusicDataView::MusicDataView(QWidget * parent)
    : QTreeWidget{parent}
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested,
            this, &MusicDataView::showContextMenu);

}

void MusicDataView::buildFrom(MusicData * musicData, SortMode mode){
    clear();
    m_parentNodes.clear();
    setColumnCount(1);

    int num_tracks = musicData->songCount();
    for (int i = 0; i < num_tracks; i++)
    {
        Song song = musicData->getSong(i);

        const QString key = makeKey(song, mode).trimmed().toLower();

        QTreeWidgetItem *parent = m_parentNodes.value(key, nullptr);
        if (!parent) {
            parent = new QTreeWidgetItem(this);
            parent->setText(0, key);
            m_parentNodes.insert(key, parent);
        }

        auto *track = new QTreeWidgetItem(parent);
        track->setText(0, song.title);
        addSongData(track, song);
    }
}


void MusicDataView::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = itemAt(pos);
    if (!item)
        return;   //right-clicked empty space

    QMenu menu(this);
    QAction *newPlAction = menu.addAction(tr("New playlist from selection"));
    menu.addSeparator();
    QAction *appendAction = menu.addAction(tr("Append to current playlist"));

    QAction *chosen = menu.exec(mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == newPlAction)
        emit newPlaylistRequested(item);
    else if (chosen == appendAction)
        emit appendToCurrentPlaylistRequested(item);
}

QString MusicDataView::makeKey(const Song song, SortMode mode){
    QString itemGenre = song.genre;
    QString itemAlbum  = song.album;
    QString itemArtist = song.artist;

    switch (mode) {
    case SortMode::AlbumOnly:
        return itemAlbum;
    case SortMode::AlbumArtist:
        return  itemArtist + " - " + itemAlbum;
    case SortMode::AlbumGenre:
        return itemGenre + " | " + itemAlbum;
    case SortMode::ArtistAlbumGenre:
        return itemGenre + " - " + itemArtist + " - " + itemAlbum;
    }
    return {};
}

void MusicDataView::addSongData(QTreeWidgetItem * item, Song song)
{
    item->setText(0, song.title);
    item->setData(0, SongRoles::Id, song.id);
    item->setData(0, SongRoles::Path, song.path);
    item->setData(0, SongRoles::Artist, song.artist);
    item->setData(0, SongRoles::Album, song.album);
    item->setData(0, SongRoles::Duration, song.duration);
    item->setData(0, SongRoles::Genre, song.genre);
    item->setData(0, SongRoles::Title, song.title);
    return;
}