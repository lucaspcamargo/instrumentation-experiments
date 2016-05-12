#ifndef QDSERIALPORT_H
#define QDSERIALPORT_H

#include <QObject>

class QSerialPort;

class QDSerialPort : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(QObject * serialPort READ serialPort NOTIFY serialPortChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastLine READ lastLine NOTIFY lastLineChanged)
public:
    explicit QDSerialPort(QObject *parent = 0);

    enum State {
        STATE_UNKNOWN,
        STATE_OPEN,
        STATE_CLOSED,
        STATE_ERROR
    };

    QString portName() const
    {
        return m_portName;
    }

    int baudRate() const
    {
        return m_baudRate;
    }

    State state() const
    {
        return m_state;
    }

    QObject * serialPort() const
    {
        return (QObject *) m_serialPort;
    }

    QString lastLine() const
    {
        return m_lastLine;
    }

signals:

    void readyRead();
    void readyReadLine();

    void portNameChanged(QString portName);

    void baudRateChanged(int baudRate);

    void stateChanged(State state);

    void serialPortChanged(QObject * serialPort);


    void lastLineChanged(QString lastLine);

public slots:
    bool open();

    bool canReadLine();
    int bytesAvailable();

    QString readLine();

    void setPortName(QString portName)
    {
        if (m_portName == portName)
            return;

        m_portName = portName;
        emit portNameChanged(portName);
    }

    void setBaudRate(int baudRate);

    static int getPortCount();
    static QString getPortNameByIndex(int index);

    void onPortReadyRead();

private:
    void setState(State s);

    QSerialPort * m_serialPort;
    QString m_portName;
    int m_baudRate;
    State m_state;
    QString m_lastLine;
};

#endif // QDSERIALPORT_H
