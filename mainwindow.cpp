#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "musicDataView.h"
#include <QLayout>
#include <qcoreevent.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include "playlist_tabbar.h"
#include <QToolButton>
#include <QPixmap>
#include <covercache.h>
#include <QPainter>

//To Do
//1. User selects music dir
//2. user makes playlist

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QSettings settings;
    //settings.clear();
    ui->setupUi(this);

    ui->pauseButton->setFocusPolicy(Qt::NoFocus);

    ui->menuBar->show();
    ui->menuFile->show();

    ui->currentArt->setVisible(false);
    ui->musicDataBar->setVisible(false);

    //default is tab focus. for lineedit, so focus out occurs when it should
    setFocusPolicy(Qt::ClickFocus);
    //ui->playlistTabs->setFocusPolicy(Qt::ClickFocus); //default is tab focus. for lineedit, so focus out occurs when it should
    //ui->playlistTabs->tabBar()->setFocusPolicy(Qt::ClickFocus);

    currentGroupMode = settings.value("GroupMode").toInt();

    ui->sortOptionsBox->setCurrentIndex(currentGroupMode);

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    PlaylistTabBar * bar = ui->playlistTabs->plTabBar();
    bar->installEventFilter(this);

    connect(bar, &PlaylistTabBar::tabDoubleClicked,
            this, &MainWindow::startRenameTab);

    ui->playlistTabs->setTabsClosable(true);


    musicData = new MusicData(this);

    //Show progress bar when collecting music
    connect(musicData, &MusicData::scanProgress, this, [this](int done, int total) {
        ui->musicDataBar->setMaximum(total);
        ui->musicDataBar->setValue(done);
        ui->musicDataBarLabel->setText(QString::number(done) + " / " + QString::number(total) + " files processed");
        ui->musicDataBarLabel->setVisible(true);
        ui->musicDataBar->setVisible(total > 0);
    });
    connect(musicData, &MusicData::finished, this, [this]() {
        ui->musicDataBar->setVisible(false);
        ui->musicDataBarLabel->setVisible(false);
    });



    ui->timeSlider->setRange(0, 0);
    ui->timeSlider->setValue(0);

    ui->timeLabel->setText("00:00");

    ui->volumeSlider->setRange(0, 100);
    float volume = 0.3;
    if (settings.contains("volume"))
        volume = settings.value("volume").toFloat();
    audioOutput->setVolume(volume);
    ui->volumeSlider->setValue(static_cast<int>(volume * 100));

    //Change text of pause button
    connect(player, &QMediaPlayer::playingChanged, this, [this](){
        if (player->isPlaying())
            ui->pauseButton->setText("Pause");
        else
            ui->pauseButton->setText("Play");
    });




    //Load musicDataView when musicData is done
    connect(musicData, &MusicData::finished, this, [this](){
        QSettings settings;
        currentGroupMode = settings.value("GroupMode").toInt();
        ui->musicDataView->buildFrom(musicData,
                                     static_cast<GroupMode>(currentGroupMode));
        //Load Playlists only on startup
        if (!startedUp)
        {
            startUp();
            startedUp = true;
        }
    });

    music_dir = settings.value("music_dir").toString();
    qDebug() << "Music dir on startup: " + music_dir;
    settings.setValue("music_dir", music_dir);

    if (!music_dir.isEmpty()) {
        musicData->buildFrom(music_dir, /*update=*/false);
    }
    else
    {
        statusBar()->showMessage(tr("Open a music folder to get started (File → Open Music Folder)"));
    }


    // CONNECTIONS --------------------------------------------------------------------------------------------


    //Pause or play button
    connect(ui->pauseButton,  &QPushButton::clicked, this, &MainWindow::pauseButtonClicked);

    //Stop Button
    connect(ui->stopButton,  &QPushButton::clicked, this, &MainWindow::stopButtonClicked);

    //Previous song button
    connect(ui->prevButton,  &QPushButton::clicked, this, &MainWindow::prevButtonClicked);

    //Next song button
    connect(ui->nextButton,  &QPushButton::clicked, this, &MainWindow::nextButtonClicked);

    //MDV sorting option options
    for (const auto &[mode, label] : kGroupModes)
        ui->sortOptionsBox->addItem("Sort by " + label, QVariant::fromValue(int(mode)));

    //Sorts MDV
    connect(ui->sortOptionsBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) return;   //placeholder text
        const int raw = ui->sortOptionsBox->itemData(index).toInt();
        currentGroupMode = raw;
        QSettings settings;
        settings.setValue("GroupMode", currentGroupMode);
        ui->musicDataView->buildFrom(musicData, static_cast<GroupMode>(raw));
    });

    //When user presses slider, give user priority
    connect(ui->timeSlider, &QSlider::sliderPressed, this, [this]() {
        userSeeking = true;
        qDebug() << userSeeking;
    });

    //When user releases slide, release control and set player position
    connect(ui->timeSlider, &QSlider::sliderReleased, this, [this]() {
        userSeeking = false;
        player->setPosition(ui->timeSlider->value());
    });

    //Change time when slider moves
    connect(ui->timeSlider, &QSlider::sliderMoved, this, [this](int value) {
        ui->timeLabel->setText(formatTime(value) + " / " + formatTime(player->duration()));
    });

    //Change slider pos while media progresses
    connect(player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        if (!userSeeking) {
            ui->timeSlider->setValue(static_cast<int>(position));
        }
        ui->timeLabel->setText(formatTime(position) + " / " + formatTime(player->duration()));
    });

    //Change volume
    connect(ui->volumeSlider, &QSlider::valueChanged, this, [this]()
    {
        float volume = static_cast<float>(ui->volumeSlider->value()) / 100;
        QSettings volume_settings;
        audioOutput->setVolume((volume));
        volume_settings.setValue("volume", volume);
    });

    //Close playlist tab
    connect(ui->playlistTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget *page = ui->playlistTabs->widget(index);
        ui->playlistTabs->removeTab(index);
        Playlist *playlist = page->findChild<Playlist *>();
        playlist->removePlaylist();
        page->deleteLater();
    });

    //Make new empty playlist
    //Need tose lambda cause QT checks for number of parameter, despite defaults, when using connect like this
    connect(ui->playlistTabs, &PlaylistTabWidget::emptyAreaDoubleClicked,
            this, [this]() { createPlaylistTab(QString(), false); });

    //MusicDataView node selected
    connect(ui->musicDataView, &QTreeWidget::itemDoubleClicked, this, &MainWindow::musicDataSelected);

    //Make new playlist
    connect(ui->musicDataView, &MusicDataView::newPlaylistRequested,
            this, &MainWindow::musicDataSelected);

    //Append to current playlist
    connect(ui->musicDataView, &MusicDataView::appendToCurrentPlaylistRequested,
            this, &MainWindow::appendSongsToCurrentPlaylist);


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::pauseButtonClicked()
{
    if (player->isPlaying())
        player->pause();
    else
        player->play();
}

void MainWindow::stopButtonClicked()
{
    if (player->hasAudio())
    {
        player->stop();
        ui->currentArt->setVisible(false);
        ui->timeSlider->setMaximum(0);
        ui->timeLabel->setText("00:00");
    }
}

void MainWindow::prevButtonClicked()
{
    Playlist *playlist = currentPlaylist();
    int row = playlist->currentRow();
    int total_rows = playlist->rowCount();
    int new_row = (row - 1) % total_rows;
    playlist->setCurrentCell(new_row, 0);
    playlistSongSelected();
}

void MainWindow::nextButtonClicked()
{
    Playlist *playlist = currentPlaylist();
    int row = playlist->currentRow();
    int total_rows = playlist->rowCount();
    int new_row = (row + 1) % total_rows;
    playlist->setCurrentCell(new_row, 0);
    playlistSongSelected();
}

void MainWindow::startUp()
{
    QSettings settings;
    QString path = settings.value("currently_playing").toString();
    QString current_playlist = settings.value("current_playlist").toString();

    const Song * song = musicData->getSong(path);

    QSettings pl_settings;
    //pl_settings.clear();
    pl_settings.beginGroup("playlists");
    const QStringList names = pl_settings.allKeys();
    pl_settings.endGroup();

    for (const QString &name : names)
    {
        createPlaylistTab(name, true);
    }


    if (names.contains(current_playlist))
    {
        //set current tab to current_playlist
        for (int i = 0; i < ui->playlistTabs->count(); ++i)
        {
            const int current_row = settings.value("current_row").toInt();
            if (ui->playlistTabs->tabText(i) == current_playlist)
            {
                ui->playlistTabs->setCurrentIndex(i);

                //set current song if possible
                Playlist * starting_pl = currentPlaylist();
                const Song * starting_song = starting_pl->getSong(current_row);
                if (starting_song && starting_song->path == path)
                {
                    starting_pl->setCurrentCell(current_row, 0);
                    playlistSongSelected(false);
                }
                break;
            }
        }
    }
    else if (song)
    {
        createPlaylistTab(song->title, false);
        Playlist * pl = currentPlaylist();
        pl->appendSong(*song, true);
    }
}

void MainWindow::playlistSongSelected(bool play)
{
    QSettings defaults;
    Playlist *playlist = currentPlaylist();
    if (!playlist) return;

    const Song * song = playlist->getSong(playlist->currentRow());
    if (!song) return;

    int duration = song->duration;
    player->setSource(QUrl::fromLocalFile(song->path));

    ui->timeSlider->setMaximum((duration));
    ui->timeLabel->setText("00:00 / " + formatTime(duration));
    if (play)
        player->play();
    int width = ui->currentArt->width();
    int height = ui->currentArt->height();
    QImage cover;
    cover.loadFromData(CoverCache::load(song->coverKey));
    const QPixmap pix = QPixmap::fromImage(cover);

    QPixmap canvas(width, height);
    canvas.fill(Qt::black);

    // Scale image preserving aspect ratio, then center it on the canvas
    QPixmap scaled = pix.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPainter painter(&canvas);
    int x = (width - scaled.width()) / 2;
    int y = (height - scaled.height()) / 2;
    painter.drawPixmap(x, y, scaled);
    painter.end();

    ui->currentArt->setPixmap(canvas);
    ui->currentArt->setVisible(true);

    defaults.setValue("currently_playing", song->path);
    defaults.setValue("current_playlist", playlist->getName());
    defaults.setValue("current_row", playlist->currentRow());
}

void MainWindow::musicDataSelected(QTreeWidgetItem * node)
{
    //QTreeWidgetItem * node = ui->musicDataView->selectedItems().at(0);
    QString playlist_name = node->text(0);
    Playlist * p = createPlaylistTab(playlist_name);
    p->appendNodeChildSongs(node);
}

void MainWindow::appendSongsToCurrentPlaylist(QTreeWidgetItem * node)
{
    Playlist *playlist = currentPlaylist();
    playlist->appendNodeChildSongs(node);
}


Playlist *MainWindow::createPlaylistTab(const QString &name, bool load)
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    //page->setFocusPolicy(Qt::NoFocus);

    auto *playlist = new Playlist(page);
    playlist->buildFrom(musicData, name, false);
    layout->addWidget(playlist);

    ui->playlistTabs->addTab(page, playlist->getName());
    ui->playlistTabs->setCurrentWidget(page);

    //Double click to select song per playlist
    connect(playlist, &QTableWidget::cellDoubleClicked,
            this, [this](int, int) { playlistSongSelected(); });

    return playlist;
}



Playlist *MainWindow::currentPlaylist() const
{
    QWidget *page = ui->playlistTabs->currentWidget();
    return page ? page->findChild<Playlist *>() : nullptr;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (auto *editor = qobject_cast<QLineEdit *>(watched)) {
        if (event->type() == QEvent::FocusOut) {
            //Lost focus
            emit editor->editingFinished();
            return false;
        }
        if (event->type() == QEvent::KeyPress) {
            auto *ke = static_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_Escape) {
                emit editor->editingFinished();
                return true;
            }
        }

    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::startRenameTab(int index)
{
    PlaylistTabBar *bar = ui->playlistTabs->plTabBar();
    if (index < 0 || index >= bar->count())
        return;
    if (bar->getEdittingTab() != -1)
        return;   // already editing

    bar->setEdittingTab(index);

    auto *editor = new QLineEdit(bar->tabText(index), bar); //make a lineedit consisting of the tab taxt, make bar it's parent
    editor->selectAll();

    // Put the editor on the right side of the tab, and hide the tab's text.
    bar->setTabButton(index, QTabBar::RightSide, editor);
    QString old = bar->tabText(index);
    bar->setTabText(index, QString());   // hide the label while editing

    // Commit on Enter or focus loss; cancel on Escape.
    connect(editor, &QLineEdit::editingFinished, this, [this, editor, index, bar, old]() {
        commitRename(editor, index, bar, old);   // no-op if already committed
    });


    // Escape — QLineEdit doesn't have a signal for this, so filter it.
    editor->installEventFilter(this);

    editor->setFocus();
}

void MainWindow::commitRename(QLineEdit *editor, int index, PlaylistTabBar *bar, QString old)
{
    if (!editor)
        return;

    const QString newName = editor->text().trimmed();

    // Restore the normal tab layout first.
    bar->setTabButton(index, QTabBar::RightSide, nullptr); //removes editor from position
    bar->setTabText(index, newName.isEmpty() ? old : newName);
    bar->setEdittingTab(-1);
    editor->deleteLater();

    // Update the playlist object and re-save.
    QWidget *page = ui->playlistTabs->widget(index);
    if (page) {
        if (auto *playlist = page->findChild<Playlist *>()) {
            playlist->setName(newName, true);
        }
    }
}

QString MainWindow::formatTime(qint64 ms)
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

void MainWindow::on_actionOpen_Music_Folder_triggered()
{
    QSettings settings;
    music_dir = QFileDialog::getExistingDirectory(this, tr("Music Folder"),
                                                  "/home/smart/Music/Music (saved)",
                                                  QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);

    if (!music_dir.isEmpty())
        musicData->buildFrom(music_dir, true);
}



