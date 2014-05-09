#include "mainwindow.h"
#include <QApplication>
#include "notes.h"
int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Phil Wise");
    QCoreApplication::setOrganizationDomain("phil-wise.com");
    QCoreApplication::setApplicationName("qnv");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
