#include "qdaudioserialport.h"
#include "qserialport.h"

#include <QtMath>
#include <complex>

QDAudioSerialPort::QDAudioSerialPort(QObject *parent):
  QDSerialPort(parent),
  m_frequency(0.0),
  m_cutoff(false),
  m_bufferSize(AUDIO_BUFFER_SIZE),
  m_level(0),
  m_fftLevel(0),
  m_sampleRate(AUDIO_SAMPLE_RATE),
  m_fftPeakIndex(0),
  m_fftPeakFrequency(0),
  m_hannWindowEnabled(true),
  m_normalizationEnabled(false)
{
    connect(this, &QDSerialPort::readyRead, this, &QDAudioSerialPort::onSuperReadyRead);
    initFFT();
}

void QDAudioSerialPort::onSuperReadyRead()
{
    while(bytesAvailable() >= AUDIO_BUFFER_SIZE )
    {
        QSerialPort * port = (QSerialPort*) serialPort();
        if(port && port->isOpen())
        {
            port->read((char*)buffer, AUDIO_BUFFER_SIZE);
            processBuffer();
        }
    }

}

void QDAudioSerialPort::initFFT()
{
    m_fft_in = fftw_alloc_real(AUDIO_BUFFER_SIZE);
    m_fft_out = fftw_alloc_complex(AUDIO_BUFFER_SIZE);
    Q_ASSERT(m_fft_in && m_fft_out);

    for (int a = 0; a < AUDIO_BUFFER_SIZE; a++) {
        m_fft_in[a] = 0;
        m_fft_out[a][0] = 0;
        m_fft_out[a][1] = 0;
    }

    m_fft_plan = fftw_plan_dft_r2c_1d(AUDIO_BUFFER_SIZE, m_fft_in, m_fft_out, 0);
    Q_ASSERT(m_fft_plan);
}

void QDAudioSerialPort::processBuffer()
{
    bool cutoff = false;

    int maxRange = qMin(255 - AUDIO_ZERO_LEVEL, AUDIO_ZERO_LEVEL);
    float floatScale = 1.0f / maxRange;

    float level = 0.0f;
    bool positive = false;
    int zeroPasses = 0;

    for(int i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        m_fft_in[i] = (bufferScaled[i] = floatScale * (static_cast<int>(buffer[i]) - AUDIO_ZERO_LEVEL));
        level = qMax(level, qAbs(bufferScaled[i]));

        if( (bufferScaled[i] > 0) != positive)
        {
            positive = !positive;
            zeroPasses++;
        }

        if(buffer[i] > CUTOFF_THRESHOLD)
            cutoff = true;
    }

    if(m_normalizationEnabled)
    {
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            bufferScaled[i] /= level;
        }
    }

    if(m_hannWindowEnabled)
    {
        applyHannWindow();
    }

    // fft part

    fftw_execute(m_fft_plan);

    double fftLevel = 0.0;
    int fftMaxIndex = 0;

    for(int i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        m_fft_in[i] = std::abs(std::complex<double>(m_fft_out[i][0], m_fft_out[i][1]));
        if(m_fft_in[i] > fftLevel)
        {
            fftLevel = m_fft_in[i];
            fftMaxIndex = i;
        }

    }

    setFrequency(zeroPasses * 2);
    setLevel(level);
    setFftLevel(fftLevel);
    setFftPeakFrequency(fftMaxIndex * AUDIO_SAMPLE_RATE / AUDIO_BUFFER_SIZE);
    setFftPeakIndex(fftMaxIndex);
    setCutoff(cutoff);

    emit processed();
}

void QDAudioSerialPort::applyHannWindow()
{
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        double multiplier = 0.5 * (1 - qCos(M_2_PI*i/(AUDIO_BUFFER_SIZE - 1)));
        m_fft_in[i] = multiplier * m_fft_in[i];
    }
}

