#include "serialcamerawindow.h"
#include "ui_serialcamerawindow.h"

#include "serialcamera.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

SerialCameraWindow::SerialCameraWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SerialCameraWindow)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(m_scene = new QGraphicsScene(this));


    ui->gridLayout->removeWidget(ui->overlay);
    ui->gridLayout->addWidget(ui->overlay, 0, 0, 0);

    m_cam = new SerialCamera(this);
    connect(m_cam, &SerialCamera::statusChanged, this, &SerialCameraWindow::onStatusChanged);
    connect(m_cam, &SerialCamera::newFrame, this, &SerialCameraWindow::onNewFrame);
    onStatusChanged(m_cam->status());
}

SerialCameraWindow::~SerialCameraWindow()
{
    delete ui;
}

void SerialCameraWindow::onNewFrame(QImage img)
{
    m_scene->clear();
    m_scene->addPixmap(QPixmap::fromImage(img))->setTransformationMode(Qt::SmoothTransformation);
    m_scene->setSceneRect(img.rect());
    ui->graphicsView->fitInView(img.rect(), Qt::KeepAspectRatio);

    m_lastFrame = img;
}

void SerialCameraWindow::onStatusChanged(QString status)
{
    ui->label->setText(status);
}

#include <QDateTime>

void SerialCameraWindow::on_photoButton_clicked()
{
    if(!m_lastFrame.isNull())
    {
        QString filename = QStringLiteral("out_%1.png").arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        m_lastFrame.save(filename);

    }
}
