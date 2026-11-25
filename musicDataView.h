#ifndef MUSICDATAVIEW_H
#define MUSICDATAVIEW_H

#include <QTreeWidget>
#include "musicData.h"

class TrackItem : public QTreeWidgetItem
{
public:
    static constexpr int kSortColumn = 0;
    using QTreeWidgetItem::QTreeWidgetItem;

    bool operator<(const QTreeWidgetItem &other) const override
    {
        const int a = data(0, SongRoles::TrackNumber).toInt();
        const int b = other.data(0, SongRoles::TrackNumber).toInt();
        if (a != b)
            return a < b;
        // tie-breaker (same or missing track number): fall back to title
        return text(0).localeAwareCompare(other.text(0)) < 0;
    }
};

class MusicDataView: public QTreeWidget
{
    Q_OBJECT
public:
    explicit MusicDataView(QWidget * parent = nullptr);
    void buildFrom(MusicData * musicData, GroupMode mode);


signals:
    void newPlaylistRequested(QTreeWidgetItem * node);
    void appendToCurrentPlaylistRequested(QTreeWidgetItem * node);

private slots:
    void showContextMenu(const QPoint &pos);

private:
    QString makeKey(const Song song, GroupMode mode);
    void addSongData(QTreeWidgetItem * item, Song song);
    QHash<QString, QTreeWidgetItem *> m_parentNodes;
    int static const PARENT_ITEM_SIZE = 40;
    int static const CHILD_ITEM_SIZE = 25;
    int static const ITEM_WIDTH = 500;
    int static const ICON_SIZE = 32;



};


#endif // MUSICDATAVIEW_H
