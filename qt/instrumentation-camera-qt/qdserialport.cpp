#include "qdserialport.h"

#include <QSerialPort>
#include <QSerialPortInfo>

QDSerialPort::QDSerialPort(QObject *parent) :
  QObject(parent),
  m_serialPort(0),
  m_portName(""),
  m_baudRate(9600),
  m_state(STATE_CLOSED)
{

}

bool QDSerialPort::open()
{
    if(m_serialPort)
    {
        return m_state == STATE_OPEN;
    }

    QSerialPort * newPort = new QSerialPort(m_portName, this);
    newPort->setBaudRate(m_baudRate);
    connect(newPort, &QSerialPort::readyRead, this, &QDSerialPort::onPortReadyRead);

    if(newPort->open(QIODevice::ReadWrite))
    {
        m_serialPort = newPort;
        setState(STATE_OPEN);
        emit serialPortChanged(newPort);

        return true;
    }
    else
    {
        setState(STATE_ERROR);
        delete newPort;

        return false;
    }
}

bool QDSerialPort::canReadLine()
{
    if(m_serialPort)
    {
        return m_serialPort->canReadLine();
    }

    else return false;
}

int QDSerialPort::bytesAvailable()
{
    if(m_serialPort)
    {
        return m_serialPort->bytesAvailable();
    }

    else return 0;
}

void QDSerialPort::sendChar(char c)
{
    if(m_serialPort)
    {
        m_serialPort->write((char*)&c, 1);
    }
}



QString QDSerialPort::readLine()
{
    QString str;

    if(m_serialPort)
    {
        str = m_serialPort->readLine();
        m_lastLine = str;
        emit lastLineChanged(m_lastLine);
    }

    return str;

}

void QDSerialPort::flush()
{
    if(m_serialPort) m_serialPort->flush();
}

char QDSerialPort::readChar()
{
    char ret = '\0';

    if(m_serialPort)
        if(m_serialPort->bytesAvailable())
            m_serialPort->read((char*)&ret, 1);

    return ret;
}

QByteArray QDSerialPort::readBytes(quint64 count)
{
    if(!m_serialPort) return QByteArray();
    return m_serialPort->read(count);
}

void QDSerialPort::setBaudRate(int baudRate)
{
    if (m_baudRate == baudRate)
        return;

    m_baudRate = baudRate;
    emit baudRateChanged(baudRate);

    if(m_serialPort)
        m_serialPort->setBaudRate(baudRate);
}

int QDSerialPort::getPortCount()
{
    return QSerialPortInfo::availablePorts().size();
}

QString QDSerialPort::getPortNameByIndex(int index)
{
    return QSerialPortInfo::availablePorts().at(index).portName();
}

void QDSerialPort::onPortReadyRead()
{
    emit readyRead();
    if( canReadLine() ) emit readyReadLine();
}

void QDSerialPort::setState(QDSerialPort::State s)
{
    if(s == m_state) return;

    m_state = s;

    emit stateChanged(s);
}

