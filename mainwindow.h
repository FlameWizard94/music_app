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
#include "musicData.h"
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
    void pauseButtonClicked(bool checked);
    void playlistSongSelected();
    void on_actionOpen_Music_Folder_triggered();
    void musicDataSelected(QTreeWidgetItem * node, int column);

private:
    Ui::MainWindow *ui;
    QString music_dir;
    QPushButton * pauseButton;
    QMediaPlayer *player;
    QAudioOutput * audioOutput;
    MusicData * musicData;
    //Playlist * playlist;
    bool userSeeking = false;
    Playlist *createPlaylistTab(const QString &title);
    Playlist *currentPlaylist() const;
    QString formatTime(qint64 ms);
};
#endif // MAINWINDOW_H
