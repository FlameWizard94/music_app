#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "musicDataView.h"
#include <QLayout>

//To Do
//1. User selects music dir
//2. user makes playlist

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pauseButton->setCheckable(true);

    ui->menuBar->show();
    ui->menuFile->show();

    QSettings settings;
    music_dir = settings.value("music_dir").toString();
    qDebug() << "Music dir on startup: " + music_dir;
    settings.setValue("music_dir", music_dir);


    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    //player->setSource(QUrl::fromLocalFile("/home/smart/Documents/Projects/Music Application/testing/Track 3.m4a"));

    ui->playlistTabs->setTabsClosable(true);


    musicData = new MusicData(this);
    if (!music_dir.isEmpty()) {
        musicData->buildFrom(music_dir, /*update=*/false);
        ui->musicDataView->buildFrom(musicData,
                                     SortMode::AlbumArtist);
    }
    else
    {
        statusBar()->showMessage(tr("Open a music folder to get started (File → Open Music Folder)"));
    }

    ui->sortOptionsBox->setPlaceholderText("Select sorting option");
    for (const auto &[mode, label] : kSortModes)
        ui->sortOptionsBox->addItem("Sort by " + label, QVariant::fromValue(int(mode)));

    connect(ui->sortOptionsBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) return;   //placeholder text
        const int raw = ui->sortOptionsBox->itemData(index).toInt();
        ui->musicDataView->buildFrom(musicData, static_cast<SortMode>(raw));
    });


    ui->timeSlider->setRange(0, 0);
    ui->timeSlider->setValue(0);

    ui->timeLabel->setText("00:00");

    ui->volumeSlider->setRange(0, 100);
    float volume = settings.value("volume").toFloat();
    audioOutput->setVolume(volume);
    ui->volumeSlider->setValue(static_cast<int>(volume * 100));

    //Change text of pause button
    connect(player, &QMediaPlayer::playingChanged, this, [this](){
        if (player->isPlaying())
            ui->pauseButton->setText("Pause");
        else
            ui->pauseButton->setText("Play");
    });

    //Pause or play button
    connect(ui->pauseButton,  &QPushButton::clicked, this, &MainWindow::pauseButtonClicked);


    //Load musicDataView when musicData is done
    connect(musicData, &MusicData::finished, this, [this](){
        ui->musicDataView->buildFrom(musicData,
                                     SortMode::AlbumArtist);
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
        page->deleteLater();
    });

    //MusicDataView node selected
    connect(ui->musicDataView, &QTreeWidget::itemDoubleClicked, this, &MainWindow::musicDataSelected);


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::pauseButtonClicked(bool checked)
{
    if (checked)
    {
        player->play();
        ui->pauseButton->setText("Pause");
    }
    else
    {
        player->pause();
        ui->pauseButton->setText("Play");
    }
}

void MainWindow::musicDataSelected(QTreeWidgetItem * node, int column)
{
    //QTreeWidgetItem * node = ui->musicDataView->selectedItems().at(0);
    QString playlist_name = node->text(0);
    Playlist * p = createPlaylistTab(playlist_name);
    int num_children = node->childCount();
    if(num_children > 0)
    {
        for (int i = 0; i < num_children; i++)
        {
            p->appendSong(node->child(i));
        }
    }
    else
    {
        p->appendSong(node);
    }
}

void MainWindow::playlistSongSelected()
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
    player->play();
    qDebug() << "Duration " << duration;
}


Playlist *MainWindow::createPlaylistTab(const QString &title)
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *playlist = new Playlist(page);
    playlist->linkToData(musicData);
    layout->addWidget(playlist);

    ui->playlistTabs->addTab(page, title);
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

    musicData->buildFrom(settings.value("music_dir").toString(), true);
}



