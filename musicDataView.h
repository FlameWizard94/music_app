#ifndef MUSICDATAVIEW_H
#define MUSICDATAVIEW_H

#include <QTreeWidget>
#include "musicData.h"

enum class SortMode { AlbumOnly, AlbumArtist, AlbumGenre, ArtistAlbumGenre };

class MusicDataView: public QTreeWidget
{
    Q_OBJECT
public:
    explicit MusicDataView(QWidget * parent = nullptr);
    void buildFrom(MusicData * musicData, SortMode mode);

private:
    QString makeKey(const Song song, SortMode mode);
    void addSongData(QTreeWidgetItem * item, Song song);
    QHash<QString, QTreeWidgetItem *> m_parentNodes;

};


#endif // MUSICDATAVIEW_H
