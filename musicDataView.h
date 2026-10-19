#ifndef MUSICDATAVIEW_H
#define MUSICDATAVIEW_H

#include <QTreeWidget>
#include "musicData.h"

class MusicDataView: public QTreeWidget
{
    Q_OBJECT
public:
    explicit MusicDataView(QWidget * parent = nullptr);
    void buildFrom(MusicData * musicData, SortMode mode);


signals:
    void newPlaylistRequested(QTreeWidgetItem * node);
    void appendToCurrentPlaylistRequested(QTreeWidgetItem * node);

private slots:
    void showContextMenu(const QPoint &pos);

private:
    QString makeKey(const Song song, SortMode mode);
    void addSongData(QTreeWidgetItem * item, Song song);
    QHash<QString, QTreeWidgetItem *> m_parentNodes;



};


#endif // MUSICDATAVIEW_H
