#include "serialcamerawindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SerialCameraWindow w;
    w.show();

    return a.exec();
}
