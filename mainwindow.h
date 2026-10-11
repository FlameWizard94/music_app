#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QProgressBar>
#include <QLabel>
#include <QSettings>
#include <QPushButton>
#include <QSlider>
#include "music_data.h"
#include "playlist.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_pauseButton_clicked(bool checked);
    void songSelected();
    QString formatTime(qint64 ms);

    void on_actionOpen_Music_Folder_triggered();

private:
    Ui::MainWindow *ui;
    QString music_dir;
    QPushButton * pauseButton;
    QMediaPlayer *player;
    QAudioOutput * audioOutput;
    MusicData * musicData;
    //Playlist * playlist;
    bool userSeeking = false;
};
#endif // MAINWINDOW_H
