#ifndef PLAYLIST_TABBAR_H
#define PLAYLIST_TABBAR_H

#include <QTabBar>
#include <QMouseEvent>

// playlisttabbar.h
class PlaylistTabBar : public QTabBar
{
    Q_OBJECT
public:
    using QTabBar::QTabBar;
    int getEdittingTab(){ return edittingTab; }
    void setEdittingTab(int index){ edittingTab = index; }

signals:
    void tabDoubleClicked(int index);
    void emptyAreaDoubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton) {
            const int index = tabAt(e->pos());
            if (index >= 0) {
                emit tabDoubleClicked(index);
            }
            else
            {
                emit emptyAreaDoubleClicked();
            }
            e->accept();
            return;
        }
        QTabBar::mouseDoubleClickEvent(e);
    }

private:
    int edittingTab = -1;
};


#endif // PLAYLIST_TABBAR_H
