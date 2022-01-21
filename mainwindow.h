#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
//#include </usr/local/include/opencv4/opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();




private:
    QTimer mTimer;


    Ui::MainWindow *ui;

    QGraphicsScene mSceneCamera;
    QGraphicsScene mSceneDebug;

    QGraphicsPixmapItem* mPixmapCamera;
    QGraphicsPixmapItem* mPixmapDebug;

    cv::VideoCapture mCameraStream;

    cv::Mat mCameraImage;
    cv::Mat mDebugImage;

    void setCameraImage(cv::Mat image);
    void setDebugImage(cv::Mat image);
    void colorDetection(cv::Mat image);
    void detectFields(cv::Mat debugImage);
    void insertCoins(cv::Mat cameraImage);
    int checkWin(cv::Mat cameraImage);

public slots:

    void processSingleFrame();
    void setLoopTime(int value);
    void start();
    void stop();
    void step();
    void reset();
    void calibrate();

};
#endif // MAINWINDOW_H
