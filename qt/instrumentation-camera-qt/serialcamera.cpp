#include "serialcamera.h"

#include <QDateTime>

#define CAMERA_BAUDRATE (460800)
#define SIZE_DIV 4
#define FRAME_W (640/SIZE_DIV)
#define FRAME_H (480/SIZE_DIV)
#define FRAME_D (1)
#define FRAME_BYTES (FRAME_W * FRAME_H * FRAME_D)

SerialCamera::SerialCamera(QObject * p) : QDSerialPort(p)
{
    setPortName(getPortNameByIndex(0));
    setBaudRate(CAMERA_BAUDRATE);

    open();
    setStatus(QStringLiteral("%1@%2").arg(portName()).arg(CAMERA_BAUDRATE));
    m_lastFrameMsecs = QDateTime::currentMSecsSinceEpoch();

    connect(this, &SerialCamera::readyReadLine, this, &SerialCamera::onNextLine);
    connect(this, &SerialCamera::readyRead, this, &SerialCamera::onReadyRead);

    m_readingFrame = false;

    sendChar('x'); // stop any ongoing capture
    sendChar('C'); // binary capture (buffered mode)
    flush();
}

SerialCamera::~SerialCamera()
{
    sendChar('x'); // stop capture
    flush();
}

void SerialCamera::onReadyRead()
{
    if(m_readingFrame && (bytesAvailable() >= FRAME_BYTES))
    {
        quint64 msecsNow = QDateTime::currentMSecsSinceEpoch(),
                delta    = msecsNow - m_lastFrameMsecs;

        m_lastFrameMsecs = msecsNow;

        QByteArray frame = readBytes(FRAME_BYTES);

        QImage inPlace((const uchar *)frame.data(), FRAME_W, FRAME_H, FRAME_W * FRAME_D, QImage::Format_Grayscale8);
        emit newFrame(inPlace.copy(inPlace.rect()));

        setStatus(QStringLiteral("%1@%2 - %3x%4 - %5 fps").arg(portName()).arg(CAMERA_BAUDRATE).arg(FRAME_W).arg(FRAME_H).arg(1000.0/delta, 2));


        m_readingFrame = false;

        while(canReadLine())
        {
            QString line = readLine();

            if(line.startsWith("FRAME"))
            {
                m_readingFrame = true;
                break;
            }
        }
    }
}

void SerialCamera::onNextLine()
{
    while(!m_readingFrame && canReadLine())
    {
        QString line = readLine();

        if(line.startsWith("FRAME"))
            m_readingFrame = true;
    }
}

