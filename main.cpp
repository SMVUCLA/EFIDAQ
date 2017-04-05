#include "daqwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DAQWindow w;
    w.show();

    return a.exec();
}
