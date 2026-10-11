#include "music_data.h"
#include <QListView>
#include <QListWidget>
#include <QStringList>
#include <QDirIterator>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QSettings>
#include <QDebug>

MusicData::MusicData(QWidget *parent)
    :QListWidget{parent}
{
}

void MusicData::buildFrom(const QString &dir, bool update)
{
    clear();
    QSettings settings;
    player = new QMediaPlayer(this);

    QString music_dir(dir);
    current_dir = dir;
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.flac" << "*.m4a" << "*.ogg" << "*.opus";

    QStringList saved_data = settings.value(dir).toStringList();
    if (saved_data.isEmpty() || update)
    {
        QDirIterator it(music_dir, filters, QDir::Files,
                        QDirIterator::Subdirectories);
        QMediaMetaData metaData = player->metaData();
        while (it.hasNext())
            m_pendingPaths << it.next();

        connect(player, &QMediaPlayer::mediaStatusChanged,
                this, &MusicData::onMediaStatusChanged);

        loadNext();
    }
    else{
        addItems(saved_data);
    }

}


void MusicData::loadNext()
{

    if (m_pendingPaths.isEmpty())
    {
        QSettings settings;
        //addItems(new_data);
        settings.setValue("music_dir", current_dir);
        settings.setValue(current_dir, new_data);
        emit finished();
        return;
    }

    m_currentPath = m_pendingPaths.takeFirst();
    player->setSource(QUrl::fromLocalFile(m_currentPath));
}

void MusicData::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia)
    {
        extractData(m_currentPath);
        loadNext();
    }
}


void MusicData::extractData(QString path){
    QFileInfo info(path);
    QMediaMetaData metaData = player->metaData();
    QString artist = metaData.value(QMediaMetaData::ContributingArtist).toString();
    QString album  = metaData.value(QMediaMetaData::AlbumTitle).toString();
    QString genre  = metaData.value(QMediaMetaData::Genre).toString();
    QString title  = metaData.value(QMediaMetaData::Title).toString();



    qlonglong durationMs = metaData.value(QMediaMetaData::Duration).toLongLong();

    auto *item = new QListWidgetItem(info.fileName());
    item->setData(SongRoles::Path, path);   // full path stored for playback
    item->setData(SongRoles::Duration, durationMs);
    item->setData(SongRoles::Artist, artist);
    item->setData(SongRoles::Album, album);
    item->setData(SongRoles::Title, title);
    item->setData(SongRoles::Genre, genre);

    qDebug() << item->data(SongRoles::Path);
    qDebug() << item->data(SongRoles::Duration);
    qDebug() << item->data(SongRoles::Artist);
    qDebug() << item->data(SongRoles::Album);
    qDebug() << item->data(SongRoles::Title);
    qDebug() << item->data(SongRoles::Genre);


    item->setToolTip(path);
    addItem(item);
    new_data << item->text();
}
