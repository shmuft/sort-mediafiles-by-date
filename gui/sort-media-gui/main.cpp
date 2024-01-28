#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("IvanDolgovCompany");
    // QCoreApplication::setOrganizationDomain("");
    QCoreApplication::setApplicationName("Sort Media Files");
    QCoreApplication::setApplicationVersion("0.0.3");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
