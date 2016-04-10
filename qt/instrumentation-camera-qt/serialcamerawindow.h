#ifndef SERIALCAMERAWINDOW_H
#define SERIALCAMERAWINDOW_H

#include <QMainWindow>
#include <QImage>

class SerialCamera;
class QGraphicsScene;

namespace Ui {
class SerialCameraWindow;
}

class SerialCameraWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SerialCameraWindow(QWidget *parent = 0);
    ~SerialCameraWindow();

public slots:
    void onNewFrame(QImage img);
    void onStatusChanged(QString status);


private slots:
    void on_photoButton_clicked();

private:
    Ui::SerialCameraWindow *ui;
    SerialCamera * m_cam;
    QGraphicsScene * m_scene;
    QImage m_lastFrame;
};

#endif // SERIALCAMERAWINDOW_H
