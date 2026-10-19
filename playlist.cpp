#include "playlist.h"
#include "musicData.h"
#include <QSettings>
#include <qheaderview.h>

//Get music dir and data from qsettings
//If playlist is saved in settings and update is false, get playlist data from settings
//Otherwise, make named playlist (listwidget) based on parameters
//Save playlist in settings
//playlists will have a naming scheme of "pl_{name}" to avoid potential future conflicts

//Replace album and artist with listWidget called parameters

Playlist::Playlist(QWidget *parent)
    : QTableWidget{parent}
{
    setupColumns();
}

void Playlist::setupColumns()
{
    setColumnCount(kColumnCount);
    setHorizontalHeaderLabels({ tr("Title"), tr("Artist"), tr("Album"), tr("Duration") });

    // Behavior tweaks
    //setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);   // select the whole row
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);    // read-only, user can't edit cells
    setAlternatingRowColors(true);
    setShowGrid(false);
    verticalHeader()->setVisible(false);                   // hide row numbers
    verticalHeader()->setDefaultSectionSize(22);           // row height

    // Column sizing: Title stretches, the rest fit their content
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(kColTitle,    QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(kColArtist,   QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(kColAlbum,    QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(kColDuration, QHeaderView::ResizeToContents);
}

void Playlist::buildFrom(MusicData *musicData,
                         const QString &name,
                         bool update)
{
    clearContents();
    setRowCount(0);

    m_name = name;

    if (!musicData)
        return;

    QSettings settings;
    if (!update || loadFromSettings(name)) {
        return;
    }

    m_data = musicData;

    //QStringList playlist_data;

    int num = musicData->songCount();
    for (int i = 0; i < num; ++i) {
        Song song = musicData->getSong(i);

        appendSong(song);
        //playlist_data << item->text();
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

    const int id = titleItem->data(SongRoles::Id).toInt();
    if (id < 0 || id >= m_data->songCount())
        return nullptr;

    return &m_data->getSong(id);
}

void Playlist::appendSong(const Song &song)
{
    const int row = rowCount();
    insertRow(row);

    auto *titleItem = new QTableWidgetItem(song.title);
    titleItem->setData(SongRoles::Id,       song.id);
    titleItem->setData(SongRoles::Path,     song.path);
    titleItem->setData(SongRoles::Artist,   song.artist);
    titleItem->setData(SongRoles::Album,    song.album);
    titleItem->setData(SongRoles::Duration, song.duration);
    titleItem->setData(SongRoles::Genre,    song.genre);
    titleItem->setData(SongRoles::Title,    song.title);

    setItem(row, kColTitle, titleItem);

    setItem(row, kColArtist, new QTableWidgetItem(song.artist));
    setItem(row, kColAlbum,  new QTableWidgetItem(song.album));

    //auto *durationItem = new NumericTableItem(formatDuration(song.duration));
    auto *durationItem = new QTableWidgetItem(formatTime(song.duration));
    durationItem->setData(Qt::UserRole, song.duration);   // for sorting
    durationItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setItem(row, kColDuration, durationItem);
    saveToSettings();
}


void Playlist::appendSong(QTreeWidgetItem * treeItem,
                          const QList<int> &roles)
{
    const int row = rowCount();
    insertRow(row);
    QTableWidgetItem *new_item = new QTableWidgetItem;
    for (int role : roles)
        new_item->setData(role, treeItem->data(0, role).toString());

    QString artist = new_item->data(SongRoles::Artist).toString();
    QString album = new_item->data(SongRoles::Album).toString();
    qlonglong duration = new_item->data(SongRoles::Duration).toLongLong();
    new_item->setText(treeItem->text(0));

    setItem(row, kColTitle, new_item);

    setItem(row, kColArtist, new QTableWidgetItem(artist));
    setItem(row, kColAlbum,  new QTableWidgetItem(album));

    //auto *durationItem = new NumericTableItem(formatDuration(song.duration));
    auto *durationItem = new QTableWidgetItem(formatTime(duration));
    durationItem->setData(Qt::UserRole, duration);   // for sorting
    durationItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setItem(row, kColDuration, durationItem);
    saveToSettings();
}

void Playlist::appendNodeChildSongs(QTreeWidgetItem * node)
{
    int num_children = node->childCount();
    if(num_children > 0)
    {
        for (int i = 0; i < num_children; i++)
        {
            appendSong(node->child(i));
        }
    }
    else
    {
        appendSong(node);
    }
    saveToSettings();
}

void Playlist::addSongData(QTableWidgetItem * item, Song song)
{
    int row = item->row();
    item->setText(song.title);
    item->setData(SongRoles::Id, song.id);
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
    durationItem->setData(Qt::UserRole, song.duration);   // for sorting
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
        m["id"]       = it->data(SongRoles::Id);
        m["path"]     = it->data(SongRoles::Path);
        m["title"]    = it->data(SongRoles::Title);
        m["artist"]   = it->data(SongRoles::Artist);
        m["album"]    = it->data(SongRoles::Album);
        m["genre"]    = it->data(SongRoles::Genre);
        m["duration"] = it->data(SongRoles::Duration);
        list << m;
    }
    settings.beginGroup("playlists");
    settings.setValue(m_name, list);
    settings.sync();
    settings.endGroup();
}

bool Playlist::loadFromSettings(const QString &name)
{
    QSettings settings;
    settings.beginGroup("playlists");
    const QVariantList playlist_data = settings.value(name).toList();
    settings.endGroup();
    if (playlist_data.empty() || !m_data)
        return false;

    m_name = name;
    for (const QVariant &v : playlist_data) {
        const QVariantMap m = v.toMap();
        int id = m.value("id").toInt();
        Song song = m_data->getSong(id);
        appendSong(song);
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