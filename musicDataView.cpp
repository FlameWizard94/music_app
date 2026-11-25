#include <QTreeWidget>
#include "musicData.h"
#include "musicDataView.h"
#include <QMenu>
#include <covercache.h>

//Using hashtable instead of treewidget.finditems() for better time complexity
//O(N) vs o(N*M), N = num tracks, M = num unique keys/parent nodes


MusicDataView::MusicDataView(QWidget * parent)
    : QTreeWidget{parent}
{
    setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    //setStyleSheet("QTreeWidget::item { height: 40px; }");
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested,
            this, &MusicDataView::showContextMenu);

}

void MusicDataView::buildFrom(MusicData * musicData, GroupMode mode){
    clear();
    m_parentNodes.clear();
    setColumnCount(1);

    const QHash<QString, Song> * songs = musicData->allSongs();
    int width;
    for (const auto &song : *songs)
    {
        const QString key = makeKey(song, mode).trimmed().toLower();

        if (!song.album.isEmpty())
        {
            QTreeWidgetItem *parent = m_parentNodes.value(key, nullptr);
            if (!parent) {
                parent = new QTreeWidgetItem(this);
                parent->setText(0, key);
                parent->setSizeHint(0, QSize(ITEM_WIDTH, PARENT_ITEM_SIZE));
                m_parentNodes.insert(key, parent);
                if (parent->icon(0).isNull() && !song.coverKey.isEmpty())
                {
                    if (!song.coverKey.isEmpty()) {
                        const QByteArray bytes = CoverCache::load(song.coverKey);
                        if (!bytes.isEmpty()) {
                            QImage cover;
                            if (cover.loadFromData(bytes) && !cover.isNull())
                                parent->setIcon(0, QIcon(QPixmap::fromImage(cover)));
                        }
                    }
                }
            }

            auto *track = new TrackItem(parent);
            track->setText(0, song.title);
            track->setSizeHint(0, QSize(ITEM_WIDTH, CHILD_ITEM_SIZE));
            addSongData(track, song);
        }
        else{
            continue;
        }
    }

    sortItems(0, Qt::SortOrder::AscendingOrder);

    for (int i = 0; i < topLevelItemCount(); ++i)
        topLevelItem(i)->sortChildren(TrackItem::kSortColumn, Qt::AscendingOrder);
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

QString MusicDataView::makeKey(const Song song, GroupMode mode){
    QString itemGenre = song.genre;
    QString itemAlbum  = song.album;
    QString itemArtist = song.artist;

    switch (mode) {
    case GroupMode::AlbumOnly:
        return itemAlbum;
    case GroupMode::AlbumArtist:
        return  itemArtist + " - " + itemAlbum;
    case GroupMode::AlbumGenre:
        return itemGenre + " | " + itemAlbum;
    case GroupMode::ArtistAlbumGenre:
        return itemGenre + " - " + itemArtist + " - " + itemAlbum;
    }
    return {};
}

void MusicDataView::addSongData(QTreeWidgetItem * item, Song song)
{
    item->setText(0, song.title);
    //item->setData(0, SongRoles::Id, song.id);
    item->setData(0, SongRoles::Path, song.path);
    item->setData(0, SongRoles::Artist, song.artist);
    item->setData(0, SongRoles::Album, song.album);
    item->setData(0, SongRoles::Duration, song.duration);
    item->setData(0, SongRoles::Genre, song.genre);
    item->setData(0, SongRoles::Title, song.title);
    item->setData(0, SongRoles::TrackNumber, song.track);
    return;
}