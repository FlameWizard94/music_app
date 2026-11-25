#include "playlist.h"
#include "musicData.h"
#include <QSettings>
#include <qheaderview.h>
#include <qtconcurrentmap.h>
#include <QElapsedTimer>

//Get music dir and data from qsettings
//If playlist is saved in settings and update is false, get playlist data from settings
//Otherwise, make named playlist (listwidget) based on parameters
//Save playlist in settings
//playlists will have a naming scheme of "pl_{name}" to avoid potential future conflicts

//Replace album and artist with listWidget called parameters

int Playlist::activePlaylists = 0;

Playlist::Playlist(QWidget *parent)
    : QTableWidget{parent}
{
    setupColumns();
    activePlaylists++;
    id = activePlaylists;
}

void Playlist::setupColumns()
{
    setColumnCount(kColumnCount);
    setHorizontalHeaderLabels({ tr("Track"),
                               tr("Title"),
                               tr("Artist"),
                               tr("Album"),
                               tr("Duration"),
                               tr("Date Created"),
                               tr("Date Released") });

    setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);   // select the whole row
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);    // read-only, user can't edit cells
    setAlternatingRowColors(true);
    setShowGrid(false);
    verticalHeader()->setVisible(false);                   // hide row numbers
    verticalHeader()->setDefaultSectionSize(22);           // row height

    // Column sizing: Title stretches, the rest fit their content
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(kTrackNumber,  QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(kColTitle,     QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(kColArtist,    QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(kColAlbum,     QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(kColDuration,  QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(kCreated,      QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(kRelease,      QHeaderView::ResizeToContents);
}

void Playlist::buildFrom(MusicData *musicData,
                         const QString &name,
                         bool update)
{
    clearContents();
    setRowCount(0);

    if (!name.isEmpty())
        m_name = name;
    else
        m_name = "Playlist " + QString::number(activePlaylists);

    if (musicData)
        m_data = musicData;;

    if (loadFromSettings(name) || !update) {
        return;
    }

    saveToSettings();
}

void Playlist::linkToData(MusicData * musicData) { m_data = musicData; }

void Playlist::setName(QString name, bool save)
{
    QSettings settings;
    m_name = name;
    QStringList playlist_data;

    for(int i = 0; i < rowCount(); i++)
    {
        playlist_data << item(i, 0)->text();
    }
    if (save)
        saveToSettings();
}

QString Playlist::getName(){ return m_name; }

const Song *Playlist::getSong(int row) const
{
    if (!m_data)
        return nullptr;
    if (row < 0 || row >= rowCount())
        return nullptr;

    QTableWidgetItem *titleItem = item(row, kColTitle);
    if (!titleItem)
        return nullptr;

    const QString path = titleItem->data(SongRoles::Path).toString();

    return m_data->getSong(path);
}

void Playlist::appendSong(const Song &song, bool save)
{
    const int row = rowCount();
    insertRow(row);

    auto *titleItem = new QTableWidgetItem(song.title);
    //titleItem->setData(SongRoles::Id,       song.id);
    titleItem->setData(SongRoles::Path,         song.path);
    titleItem->setData(SongRoles::Artist,       song.artist);
    titleItem->setData(SongRoles::Album,        song.album);
    titleItem->setData(SongRoles::Duration,     song.duration);
    titleItem->setData(SongRoles::Genre,        song.genre);
    titleItem->setData(SongRoles::Title,        song.title);
    titleItem->setData(SongRoles::TrackNumber,  song.track);
    titleItem->setData(SongRoles::Release,      song.release);
    titleItem->setData(SongRoles::Created,      song.created);

    setItem(row, kTrackNumber, new QTableWidgetItem( QString::number(song.track) ));

    setItem(row, kColTitle, titleItem);

    setItem(row, kColArtist, new QTableWidgetItem(song.artist));
    setItem(row, kColAlbum,  new QTableWidgetItem(song.album));

    //auto *durationItem = new NumericTableItem(formatDuration(song.duration));
    auto *durationItem = new QTableWidgetItem(formatTime(song.duration));
    durationItem->setData(Qt::UserRole, song.duration);   // for sorting
    durationItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setItem(row, kColDuration, durationItem);

    setItem(row, kCreated, new QTableWidgetItem(song.created.toString("dd/MM/yyyy")));
    setItem(row, kRelease, new QTableWidgetItem( QString::number(song.release) ));

    if (save)
        saveToSettings();
}


void Playlist::appendSong(QTreeWidgetItem * treeItem,
                          bool save,
                          const QList<int> &roles)
{
    const int row = rowCount();
    insertRow(row);
    QTableWidgetItem *new_item = new QTableWidgetItem;
    for (int role : roles)
        new_item->setData(role, treeItem->data(0, role).toString());

    QString path = new_item->data(SongRoles::Path).toString();
    const Song * song = m_data->getSong(path);


    new_item->setText(treeItem->text(0));

    setItem(row, kTrackNumber, new QTableWidgetItem( QString::number(song->track) ));

    setItem(row, kColTitle, new_item);

    setItem(row, kColArtist, new QTableWidgetItem(song->artist));
    setItem(row, kColAlbum,  new QTableWidgetItem(song->album));

    auto *durationItem = new QTableWidgetItem(formatTime(song->duration));
    durationItem->setData(Qt::UserRole, song->duration);   // for sorting
    durationItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setItem(row, kColDuration, durationItem);

    setItem(row, kCreated, new QTableWidgetItem(song->created.toString("dd/MM/yyyy")));
    setItem(row, kRelease, new QTableWidgetItem( QString::number(song->release) ));

    if (save)
        saveToSettings();
}

void Playlist::appendNodeChildSongs(QTreeWidgetItem * node)
{
    QElapsedTimer * timer = new QElapsedTimer();
    int num_children = node->childCount();
    timer->start();
    qDebug() << "Start: " << timer->durationElapsed();

    if (num_children > 0)
    {
        for (int i = 0; i < num_children; i++)
        {
            appendSong(node->child(i), false); //Don't save song after each append
            qDebug() << i << " " << timer->durationElapsed();
        }
    }
    else {
        appendSong(node);
    }
    qDebug() << "Done: " << timer->durationElapsed();
    saveToSettings(); //save whole playlist

}

void Playlist::addSongData(QTableWidgetItem * item, Song song)
{
    int row = item->row();
    item->setText(song.title);
    //item->setData(SongRoles::Id, song.id);
    item->setData(SongRoles::Path, song.path);
    item->setData(SongRoles::Artist, song.artist);
    item->setData(SongRoles::Album, song.album);
    item->setData(SongRoles::Duration, song.duration);
    item->setData(SongRoles::Genre, song.genre);
    item->setData(SongRoles::Title, song.title);

    setItem(row, kColTitle, item);
    setItem(row, kColArtist, new QTableWidgetItem(song.artist));
    setItem(row, kColAlbum,  new QTableWidgetItem(song.album));
    auto *durationItem = new QTableWidgetItem(formatTime(song.duration));
    durationItem->setData(Qt::UserRole, song.duration);
    durationItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setItem(row, kColDuration, durationItem);
    saveToSettings();
    return;
}

void Playlist::saveToSettings() const
{
    QSettings settings;
    QVariantList list;
    for (int row = 0; row < rowCount(); ++row) {
        const QTableWidgetItem *it = item(row, kColTitle);
        QVariantMap m;
        //m["id"]       = it->data(SongRoles::Id);
        m["path"]     = it->data(SongRoles::Path);
        m["title"]    = it->data(SongRoles::Title);
        m["artist"]   = it->data(SongRoles::Artist);
        m["album"]    = it->data(SongRoles::Album);
        m["genre"]    = it->data(SongRoles::Genre);
        m["duration"] = it->data(SongRoles::Duration);
        m["created"]  = it->data(SongRoles::Created);
        m["release"]  = it->data(SongRoles::Release);
        list << m;
    }
    settings.beginGroup("playlists");
    settings.setValue(m_name, list);
    settings.sync();
    settings.endGroup();
}

void Playlist::removePlaylist()
{
    QSettings settings;
    settings.beginGroup("playlists");
    settings.remove(m_name);
    activePlaylists--;
    settings.sync();
    settings.endGroup();
    deleteLater();
}

bool Playlist::loadFromSettings(const QString &name)
{
    QSettings settings;
    settings.beginGroup("playlists");
    const QVariantList playlist_data = settings.value(name).toList();
    settings.endGroup();
    if (playlist_data.empty() || !m_data)
        return false;

    int underline = name.indexOf("_") + 1;
    m_name = name.mid(underline);
    for (const QVariant &v : playlist_data) {
        const QVariantMap m = v.toMap();
        QString path = m.value("path").toString();
        const Song * song = m_data->getSong(path);
        appendSong(*song);
    }
    return true;
}

QString Playlist::formatTime(qint64 ms) const
{
    if (ms < 0) ms = 0;

    const qint64 totalSeconds = ms / 1000;
    const qint64 seconds = totalSeconds % 60;
    const qint64 totalMinutes = totalSeconds / 60;
    const qint64 minutes = totalMinutes % 60;
    const qint64 totalHours = totalMinutes / 60;
    const qint64 hours = totalHours % 24;
    const qint64 days = totalHours / 24;

    QString result;

    if (days > 0) {
        result = QString("%1:%2:%3:%4")
        .arg(days)
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    } else if (hours > 0) {
        result = QString("%1:%2:%3")
        .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    } else {
        result = QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

    return result;
}