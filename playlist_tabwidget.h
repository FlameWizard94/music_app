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
    explicit PlaylistTabWidget(QWidget *parent = nullptr)
        : QTabWidget(parent)
    {
        PlaylistTabBar * bar = new PlaylistTabBar();
        this->setTabBar(bar);

        connect(bar, &PlaylistTabBar::emptyAreaDoubleClicked,
                this, &PlaylistTabWidget::emptyAreaDoubleClicked);
    }
    ~PlaylistTabWidget(){}

    PlaylistTabBar * plTabBar()
    {
        return static_cast<PlaylistTabBar *>(this->tabBar());
    }

signals:
    void emptyAreaDoubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton) {
            const QRect barRect = tabBar()->geometry();
            // Full-width strip at the height of the tab bar
            const QRect strip(0, barRect.top(), width(), barRect.height());
            const QPoint p = e->position().toPoint();

            if (strip.contains(p) && !barRect.contains(p)) {
                emit emptyAreaDoubleClicked();
                e->accept();
                return;
            }
        }
        QTabWidget::mouseDoubleClickEvent(e);
    }

};

#endif // PLAYLIST_TABWIDGET_H
