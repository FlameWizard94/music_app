#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

//To Do
//1. User selects music dir
//2. user makes playlist

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pauseButton->setCheckable(true);


    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    //player->setSource(QUrl::fromLocalFile("/home/smart/Documents/Projects/Music Application/testing/Track 3.m4a"));

    connect(player, &QMediaPlayer::playingChanged, this, [this](){
        if (player->isPlaying())
            ui->pauseButton->setText("Pause");
        else
            ui->pauseButton->setText("Play");
    });

    connect(ui->pauseButton,  &QPushButton::clicked, this, &MainWindow::on_pauseButton_clicked);

    connect(ui->musicData, &MusicData::finished, this, [this]() {
        /*ui->playlist->buildFrom(musicData,
                                "Naoki tracks",
                                "Guilty Gear Strive OST [Vocal Tracks]",
                                "NAOKI",
                                true); */
        //connect(ui->playlist, &QListWidget::itemDoubleClicked, this, &MainWindow::songSelected);
        ui->playlist->buildFrom(ui->musicData,
                                "",
                                "",
                                "",
                                /*update=*/true);
    });

    QSettings settings;
    music_dir = settings.value("musid_dir").toString();
    qDebug() << "Music dir on startup: " + music_dir;
    settings.setValue("music_dir", music_dir);
    ui->musicData->buildFrom(music_dir,
                         /*update=*/true);


    ui->timeSlider->setRange(0, 0);
    ui->timeSlider->setValue(0);

    ui->timeLabel->setText("00:00");

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

    connect(ui->playlist, &QListWidget::itemDoubleClicked,
            this, &MainWindow::songSelected);

    ui->volumeSlider->setRange(0, 100);
    float volume = settings.value("volume").toFloat();
    audioOutput->setVolume(volume);
    ui->volumeSlider->setValue(static_cast<int>(volume * 100));

    //Change volume
    connect(ui->volumeSlider, &QSlider::valueChanged, this, [this]()
            {
                float volume = static_cast<float>(ui->volumeSlider->value()) / 100;
                QSettings volume_settings;
                audioOutput->setVolume((volume));
                volume_settings.setValue("volume", volume);
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pauseButton_clicked(bool checked)
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

void MainWindow::songSelected()
{
    QSettings defaults;
    QListWidgetItem * current = ui->playlist->currentItem();
    if (!current) return;

    QString path = current->data(SongRoles::Path).toString();
    int duration = current->data(SongRoles::Duration).toInt();
    player->setSource(QUrl::fromLocalFile(path));
    ui->timeSlider->setMaximum((duration));
    ui->timeLabel->setText("00:00 / " + formatTime(duration));
    player->play();
    qDebug() << "Duration " << duration;
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
                                                  "/home/smart/Music",
                                                  QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    settings.setValue("music_dir", music_dir);
    ui->musicData->buildFrom(settings.value("music_dir").toString(),
                         /*update=*/true);
}

