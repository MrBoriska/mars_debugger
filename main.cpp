#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();

    // эти настройки используются (неявно) классом QSettgins для
    // определения имени и местоположения конфига
    //QCoreApplication::setOrganizationName("ITUCR");
    QCoreApplication::setApplicationName("MARS");

    return a.exec();
}
