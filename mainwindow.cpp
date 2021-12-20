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
    mCameraStream.open(0);

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
/*
    cv::Mat img_gray;
    cv::cvtColor(debugImage, img_gray, cv::COLOR_BGR2GRAY);
    cv::Mat templateImage = cv::imread("/home/lluks/semester_6/Bildverarbeuitungslabor/abschlussproject/template.png", 0);
    int width = templateImage.cols;
    int height = templateImage.rows;
    cv::Mat res;
    cv::matchTemplate(img_gray, templateImage, res, cv::TM_CCOEFF_NORMED);
    double threshold = 0.8;

    //std::vector<cv::Point> data;
    for (int y=0; y < res.rows; y++) {
        for (int x=0; x < res.cols; x++) {
            if(res.at<uchar>(x, y) >= threshold){
                //cv::Point newPoint = cv::Point(x,y);
                //data.push_back(newPoint);
                //std::cout << x << ", " << y << "\n";
                cv::rectangle(debugImage, cv::Rect(x, y, width,  height), (0,0,255), 5);
            }
        }
    }
*/
    //bearbeiten Sie das debug bild wie sie wollen;

    cv::Mat templateImage = cv::imread("/home/lluks/semester_6/Bildverarbeuitungslabor/abschlussproject/template.png", 0);
    int width = templateImage.cols;
    int height = templateImage.rows;
    cv::Mat img = debugImage.clone();
    cv::cvtColor(debugImage, img, cv::COLOR_BGR2GRAY);
    // Apply template Matching
    cv::Mat res;
    cv::matchTemplate(img,templateImage,res,cv::TM_CCOEFF_NORMED);
    double min_val=0;
    double max_val=0;
    cv::Point min_loc;
    cv::Point max_loc;
    cv::minMaxLoc(res, &min_val, &max_val, &min_loc, &max_loc);
    cv::Point top_left = max_loc;
    double bottom_right = (top_left.x + width, top_left.y + height);
    cv::rectangle(debugImage, cv::Rect(top_left.x, top_left.y, width,  height), (0,0,255), 5);

/*
    cv::Mat templ; cv::Mat result;
    int match_method = cv::TM_CCOEFF_NORMED;
    templ = imread( "/home/lluks/semester_6/Bildverarbeuitungslabor/abschlussproject/sam.png", cv::IMREAD_COLOR );
    int result_cols = debugImage.cols - templ.cols + 1;
    int result_rows = debugImage.rows - templ.rows + 1;
    result.create( result_rows, result_cols, CV_32FC1 );
    matchTemplate( debugImage, templ, result, match_method);
    normalize( result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );
    double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
    cv::Point matchLoc;
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
    if( match_method  == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }
    rectangle(result, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );
*/
    this->setDebugImage(debugImage);

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
