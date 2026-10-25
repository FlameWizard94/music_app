#ifndef PLAYLIST_TABWIDGET_H
#define PLAYLIST_TABWIDGET_H

// Source - https://stackoverflow.com/a/27011004
// Posted by Nejat
// Retrieved 2026-10-22, License - CC BY-SA 3.0

#include <QTabBar>
#include <QTabWidget>
#include <playlist_tabbar.h>


class PlaylistTabWidget: public QTabWidget
{
    Q_OBJECT
public:
    PlaylistTabWidget(QObject *parent = 0) :
        QTabWidget((QWidget *)parent)
    {
        PlaylistTabBar * bar = new PlaylistTabBar();
        this->setTabBar(bar);
    }
    ~PlaylistTabWidget(){}

    PlaylistTabBar * plTabBar()
    {
        return static_cast<PlaylistTabBar *>(this->tabBar());
    }
};

#endif // PLAYLIST_TABWIDGET_H
