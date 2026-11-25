#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <QTableWidget>
#include <QTreeWidgetItem>
#include "musicData.h"

class Playlist : public QTableWidget
{
    Q_OBJECT
public:
    static int activePlaylists;
    ~Playlist(){}
    explicit Playlist(QWidget *parent = nullptr);
    void buildFrom(MusicData *data, const QString &name, bool update);

    void linkToData(MusicData * musidData);
    void appendSong(const Song &song,
                    bool save = true);
    void appendSong(QTreeWidgetItem * item,
                    bool save = true,
                    const QList<int> &roles = SongRoles::TrackRoles);
    void appendNodeChildSongs(QTreeWidgetItem * node);
    void setName(QString name, bool save = true);
    QString getName();
    const Song * getSong(int row) const;
    bool loadFromSettings(const QString &name);
    void removePlaylist();


private:
    static constexpr int kColumnCount = 7;
    static constexpr int kTrackNumber = 0;
    static constexpr int kColTitle    = 1;
    static constexpr int kColArtist   = 2;
    static constexpr int kColAlbum    = 3;
    static constexpr int kColDuration = 4;
    static constexpr int kCreated = 5;
    static constexpr int kRelease = 6;

    void setupColumns();
    QString formatTime(qint64 ms) const;
    void addSongData(QTableWidgetItem * item, Song song);
    void saveToSettings() const;


    MusicData * m_data = nullptr;
    QString m_name;
    int id; //so playlists can have the same names
};

#endif // PLAYLIST_H
