#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false);

    a.setApplicationName("macrokeyboard");
    a.setOrganizationName("feelx88");
    a.setOrganizationDomain("feelx88.de");

    MainWindow w;

    return a.exec();
}
