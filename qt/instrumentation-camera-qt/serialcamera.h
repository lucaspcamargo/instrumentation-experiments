#ifndef SERIALCAMERA_H
#define SERIALCAMERA_H

#include "qdserialport.h"

#include <QObject>
#include <QImage>

class SerialCamera : public QDSerialPort
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
public:
    SerialCamera(QObject * p);
    virtual ~SerialCamera();

    void onReadyRead();
    void onNextLine();

    QString status() const
    {
        return m_status;
    }

public slots:
    void setStatus(QString status)
    {
        if (m_status == status)
            return;

        m_status = status;
        emit statusChanged(status);
    }

signals:
    void newFrame(QImage img);
    void statusChanged(QString status);

private:
    bool m_readingFrame;

    quint64 m_lastFrameMsecs;

    QString m_status;
};

#endif // SERIALCAMERA_H
