#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "qdserialport.h"
#include "qdaudioserialport.h"
#include <QtQml>
#include <QIcon>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationDisplayName("qmldash");

    QIcon ico("/usr/share/icons/breeze/devices/64/audio-card.svg");
    app.setWindowIcon(ico);

    //qmlRegisterType<QDSerialPort>("qmldash", 1, 0, "SerialPort");
    qmlRegisterType<QDAudioSerialPort>("qmldash", 1, 0, "AudioSerialPort");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

