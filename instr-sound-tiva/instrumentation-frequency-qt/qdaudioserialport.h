#ifndef QDAUDIOSERIALPORT_H
#define QDAUDIOSERIALPORT_H

#include "qdserialport.h"
#include <cstdint>

#define AUDIO_ZERO_LEVEL    (125)
#define AUDIO_SAMPLE_RATE   (22050)
#define CUTOFF_THRESHOLD    (251)
#define AUDIO_BUFFER_SIZE   (AUDIO_SAMPLE_RATE/8)

#include <fftw3.h>

class QDAudioSerialPort : public QDSerialPort
{

    Q_OBJECT
    Q_PROPERTY(qreal frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(qreal level READ level WRITE setLevel NOTIFY levelChanged)
    Q_PROPERTY(qreal fftLevel READ fftLevel WRITE setFftLevel NOTIFY fftLevelChanged)
    Q_PROPERTY(qreal fftPeakFrequency READ fftPeakFrequency WRITE setFftPeakFrequency NOTIFY fftPeakFrequencyChanged)
    Q_PROPERTY(int fftPeakIndex READ fftPeakIndex WRITE setFftPeakIndex NOTIFY fftPeakIndexChanged)
    Q_PROPERTY(bool cutoff READ cutoff WRITE setCutoff NOTIFY cutoffChanged)
    Q_PROPERTY(int bufferSize READ bufferSize NOTIFY bufferSizeChanged)
    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(bool hannWindowEnabled READ hannWindowEnabled WRITE setHannWindowEnabled NOTIFY hannWindowEnabledChanged)
    Q_PROPERTY(bool normalizationEnabled READ normalizationEnabled WRITE setNormalizationEnabled NOTIFY normalizationEnabledChanged)
public:
    QDAudioSerialPort(QObject * parent = 0);

    qreal frequency() const
    {
        return m_frequency;
    }

    bool cutoff() const
    {
        return m_cutoff;
    }

    int bufferSize() const
    {
        return m_bufferSize;
    }

    qreal level() const
    {
        return m_level;
    }

    qreal fftLevel() const
    {
        return m_fftLevel;
    }

    int sampleRate() const
    {
        return m_sampleRate;
    }

    int fftPeakIndex() const
    {
        return m_fftPeakIndex;
    }

    qreal fftPeakFrequency() const
    {
        return m_fftPeakFrequency;
    }

    bool hannWindowEnabled() const
    {
        return m_hannWindowEnabled;
    }

    bool normalizationEnabled() const
    {
        return m_normalizationEnabled;
    }

signals:

    void frequencyChanged(qreal frequency);
    void processed();

    void cutoffChanged(bool cutoff);

    void bufferSizeChanged(int bufferSize);

    void levelChanged(qreal level);

    void fftLevelChanged(qreal fftLevel);

    void sampleRateChanged(int sampleRate);

    void fftPeakIndexChanged(int fftPeakIndex);

    void fftPeakFrequencyChanged(qreal fftPeakFrequency);

    void hannWindowEnabledChanged(bool hannWindowEnabled);

    void normalizationEnabledChanged(bool normalizationEnabled);

public slots:
    void onSuperReadyRead();

    void setFrequency(qreal frequency)
    {
        if (m_frequency == frequency)
            return;

        m_frequency = frequency;
        emit frequencyChanged(frequency);
    }

    void setCutoff(bool cutoff)
    {
        if (m_cutoff == cutoff)
            return;

        m_cutoff = cutoff;
        emit cutoffChanged(cutoff);
    }

    qreal sample(int index)
    {
        return bufferScaled[index];
    }

    qreal fftSample(int index)
    {
        return m_fft_in[index];
    }

    void setLevel(qreal level)
    {
        if (m_level == level)
            return;

        m_level = level;
        emit levelChanged(level);
    }

    void setFftLevel(qreal fftLevel)
    {
        if (m_fftLevel == fftLevel)
            return;

        m_fftLevel = fftLevel;
        emit fftLevelChanged(fftLevel);
    }

    void setSampleRate(int sampleRate)
    {
        if (m_sampleRate == sampleRate)
            return;

        m_sampleRate = sampleRate;
        emit sampleRateChanged(sampleRate);
    }

    void setFftPeakIndex(int fftPeakIndex)
    {
        if (m_fftPeakIndex == fftPeakIndex)
            return;

        m_fftPeakIndex = fftPeakIndex;
        emit fftPeakIndexChanged(fftPeakIndex);
    }

    void setFftPeakFrequency(qreal fftPeakFrequency)
    {
        if (m_fftPeakFrequency == fftPeakFrequency)
            return;

        m_fftPeakFrequency = fftPeakFrequency;
        emit fftPeakFrequencyChanged(fftPeakFrequency);
    }

    void setHannWindowEnabled(bool hannWindowEnabled)
    {
        if (m_hannWindowEnabled == hannWindowEnabled)
            return;

        m_hannWindowEnabled = hannWindowEnabled;
        emit hannWindowEnabledChanged(hannWindowEnabled);
    }

    void setNormalizationEnabled(bool normalizationEnabled)
    {
        if (m_normalizationEnabled == normalizationEnabled)
            return;

        m_normalizationEnabled = normalizationEnabled;
        emit normalizationEnabledChanged(normalizationEnabled);
    }

private:
    void initFFT();
    void processBuffer();
    void applyHannWindow();


    float bufferScaled[AUDIO_BUFFER_SIZE];
    uint8_t buffer[AUDIO_BUFFER_SIZE];

    qreal m_frequency;
    bool m_cutoff;
    int m_bufferSize;
    qreal m_level;

    fftw_plan m_fft_plan;
    double * m_fft_in;
    fftw_complex *m_fft_out;
    qreal m_fftLevel;
    int m_sampleRate;
    int m_fftPeakIndex;
    qreal m_fftPeakFrequency;
    bool m_hannWindowEnabled;
    bool m_normalizationEnabled;
};

#endif // QDAUDIOSERIALPORT_H
