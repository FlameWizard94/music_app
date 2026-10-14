#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("DizzyBeats");
    QCoreApplication::setApplicationName("Dizzy Beats");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
