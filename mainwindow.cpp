#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>
#include <QImage>
#include <QPixmap>
#include <opencv2/core/types.hpp>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&mTimer, &QTimer::timeout, this, &MainWindow::processSingleFrame);

    connect(ui->spinBox_loop, SIGNAL(valueChanged(int)), this, SLOT(setLoopTime(int)));
    connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(start()));
    connect(ui->pushButton_stop, SIGNAL(clicked()), this, SLOT(stop()));
    connect(ui->pushButton_step, SIGNAL(clicked()), this, SLOT(step()));


    ui->graphicsView_camera->setScene(&mSceneCamera);
    ui->graphicsView_debug->setScene(&mSceneDebug);

    mPixmapCamera = mSceneCamera.addPixmap(QPixmap());
    mPixmapDebug = mSceneDebug.addPixmap(QPixmap());

    //wenn sie mehrere Kameras haben müssen Sie hier die Kamera mit einem anderen index wählen
    //mCameraStream.open(1);
    mCameraStream = cv::VideoCapture("..\\bildverarbeitung\\testvideo.mp4");

    start();
}




MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setCameraImage(cv::Mat image)
{
    mCameraImage = image.clone();

    if(mCameraImage.type() == CV_8UC1)
    {
        cv::cvtColor(mCameraImage, mCameraImage, cv::COLOR_GRAY2RGB);
    }
    if(mCameraImage.type() == CV_8UC3)
    {
        cv::cvtColor(mCameraImage, mCameraImage, cv::COLOR_RGB2BGR);
    }
    if(mCameraImage.type() == CV_8UC4)
    {
        cv::cvtColor(mCameraImage, mCameraImage, cv::COLOR_RGBA2BGR);
    }

    QImage qimage(mCameraImage.data, image.cols, image.rows, image.cols*3, QImage::Format_RGB888);

    mPixmapCamera->setPixmap(QPixmap::fromImage(qimage));
    mPixmapCamera->setPos(0,0);
    ui->graphicsView_camera->fitInView(0,0, image.cols, image.rows, Qt::AspectRatioMode::KeepAspectRatio);
}

void MainWindow::setDebugImage(cv::Mat image)
{
    mDebugImage = image.clone();

    if(mDebugImage.type() == CV_8UC1)
    {
        cv::cvtColor(mDebugImage, mDebugImage, cv::COLOR_GRAY2RGB);
    }
    if(mDebugImage.type() == CV_8UC3)
    {
        cv::cvtColor(mDebugImage, mDebugImage, cv::COLOR_RGB2BGR);
    }
    if(mDebugImage.type() == CV_8UC4)
    {
        cv::cvtColor(mDebugImage, mDebugImage, cv::COLOR_RGBA2BGR);
    }

    QImage qimage(mDebugImage.data, image.cols, image.rows, image.cols*3, QImage::Format_RGB888);

    mPixmapDebug->setPixmap(QPixmap::fromImage(qimage));
    mPixmapDebug->setPos(0,0);
    ui->graphicsView_debug->fitInView(0,0, image.cols, image.rows, Qt::AspectRatioMode::KeepAspectRatio);
}

void MainWindow::processSingleFrame()
{
    QElapsedTimer measureTime;
    measureTime.start();

    cv::Mat cameraImage;

    mCameraStream >> cameraImage;

    this->setCameraImage(cameraImage);

    //hier müssen Sie Ihren code einbauen

    cv::Mat debugImage = cameraImage.clone();

    //bearbeiten Sie das debug bild wie sie wollen;

    // load templateImage
    cv::Mat templateImage = cv::imread("..\\bildverarbeitung\\template.png", 0);
    int width = templateImage.cols;
    int height = templateImage.rows;

    // Apply template Matching
    cv::Mat res_32f(debugImage.rows-height +1 , debugImage.cols-width +1 , CV_32FC1);
    cv::cvtColor(debugImage, debugImage, cv::COLOR_BGR2GRAY);

    cv::matchTemplate(debugImage,templateImage,res_32f,cv::TM_CCOEFF_NORMED);

    // Apply threshold
    int size = ((width + height) / 4) * 2 + 1; //force size to be odd
    cv::Mat res;

    res_32f.convertTo(res, CV_8U, 255.0);
    cv::adaptiveThreshold(res, res, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, size, -128);

    // draw rectangle around matches and mark current matche as drawn
    while (true) {
        double minval, maxval, threshold = 0.8;
        cv::Point minloc, maxloc;
        cv::minMaxLoc(res, &minval, &maxval, &minloc, &maxloc);

        if(maxval >= threshold){
           cv::rectangle(cameraImage, cv::Rect(maxloc.x, maxloc.y, width, height),(0,255,0), 5);
            cv::floodFill(res, maxloc, 0); //mark drawn blob, important!
       }
        else
            break;
    }


    this->setDebugImage(cameraImage);

    //sie können auch rechtecke oder linien direkt ins bild reinmalden

    //mSceneCamera.addItem(QGraphicsRectItem(...));

    //darunter sollte nichts geändert werden


    qint64 elapsedTime = measureTime.elapsed();

    ui->label_processingTime->setText(QString::number(elapsedTime) + " ms");
}


void MainWindow::setLoopTime(int value)
{
    mTimer.setInterval(value);
    std::cout << "Setting new interval '" << value << "'" << std::endl;
}

void MainWindow::start()
{
    mTimer.setInterval(ui->spinBox_loop->value());
    mTimer.start();
    std::cout << "Start" << std::endl;
    ui->label_status->setText("Status: Running");
}

void MainWindow::stop()
{
    mTimer.stop();
    std::cout << "Stop" << std::endl;
    ui->label_status->setText("Status: Stopped");
}

void MainWindow::step()
{
    mTimer.stop();
    mTimer.singleShot(0, this, SLOT(processSingleFrame()));
    std::cout << "Single Step" << std::endl;
    ui->label_status->setText("Status: Single Step");
}
