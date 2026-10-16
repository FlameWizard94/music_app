#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <QTableWidget>
#include <QTreeWidgetItem>
#include "musicData.h"

class Playlist : public QTableWidget
{
    Q_OBJECT
public:
    explicit Playlist(QWidget *parent = nullptr);
    void buildFrom(MusicData *data, const QString &name, bool update);

    void linkToData(MusicData * musidData);
    void appendSong(const Song &song);
    void appendSong(QTreeWidgetItem * item,
                    const QList<int> &roles = SongRoles::TrackRoles);
    void setName(QString name);
    QString getName();
    const Song * getSong(int row) const;


private:
    static constexpr int kColTitle    = 0;
    static constexpr int kColArtist   = 1;
    static constexpr int kColAlbum    = 2;
    static constexpr int kColDuration = 3;
    static constexpr int kColumnCount = 4;

    void setupColumns();
    QString formatTime(qint64 ms) const;
    void addSongData(QTableWidgetItem * item, Song song);
    void saveToSettings() const;

    MusicData * m_data = nullptr;
    QString m_name;
};

#endif // PLAYLIST_H
