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
#include "playlist_tabbar.h"

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

signals:
    void cancelEdit();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void pauseButtonClicked();
    void stopButtonClicked();
    void prevButtonClicked();
    void nextButtonClicked();
    void playlistSongSelected(bool play = true);
    void on_actionOpen_Music_Folder_triggered();
    void musicDataSelected(QTreeWidgetItem * node);
    void appendSongsToCurrentPlaylist(QTreeWidgetItem * node);
    void startRenameTab(int index);
    void commitRename(QLineEdit *editor, int index, PlaylistTabBar *bar, QString old);

private:
    Ui::MainWindow *ui;
    bool startedUp = false;
    QString music_dir;
    QPushButton * pauseButton;
    QMediaPlayer *player;
    QAudioOutput * audioOutput;
    MusicData * musicData;
    int currentGroupMode;
    bool userSeeking = false;
    Playlist *createPlaylistTab(const QString &name = "", bool load = false);
    Playlist *currentPlaylist() const;
    void startUp();
    QString formatTime(qint64 ms);
};
#endif // MAINWINDOW_H
