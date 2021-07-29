#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setOrganizationName("Phil Wise");
  QCoreApplication::setOrganizationDomain("phil-wise.com");
  QCoreApplication::setApplicationName("qnv");

  QApplication app(argc, argv);
  MainWindow mainwindow;
  mainwindow.show();
  return QApplication::exec();
}
