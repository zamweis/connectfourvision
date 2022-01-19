#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>
#include <QImage>
#include <QPixmap>
#include <opencv2/core/types.hpp>


std::vector<cv::Point> fields;
std::vector<std::vector<int>> coins;
int arraySet = 0;
int redCoins = 0;
int yellowCoins = 0;

// load templateImage
cv::Mat templateImage = cv::imread("template.png", 0);
int templateWidth = templateImage.cols;
int templateHeight = templateImage.rows;
int templateSize = ((templateWidth + templateHeight) / 4) * 2 + 1; //force size to be odd
int coinRadius = templateSize / 2;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
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
    mCameraStream = cv::VideoCapture("testvideo.mp4");
    start();

}


MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setCameraImage(cv::Mat image) {
    mCameraImage = image.clone();

    if (mCameraImage.type() == CV_8UC1) {
        cv::cvtColor(mCameraImage, mCameraImage, cv::COLOR_GRAY2RGB);
    }
    if (mCameraImage.type() == CV_8UC3) {
        cv::cvtColor(mCameraImage, mCameraImage, cv::COLOR_RGB2BGR);
    }
    if (mCameraImage.type() == CV_8UC4) {
        cv::cvtColor(mCameraImage, mCameraImage, cv::COLOR_RGBA2BGR);
    }

    QImage qimage(mCameraImage.data, image.cols, image.rows, image.cols * 3, QImage::Format_RGB888);

    mPixmapCamera->setPixmap(QPixmap::fromImage(qimage));
    mPixmapCamera->setPos(0, 0);
    ui->graphicsView_camera->fitInView(0, 0, image.cols, image.rows, Qt::AspectRatioMode::KeepAspectRatio);
}

void MainWindow::setDebugImage(cv::Mat image) {
    mDebugImage = image.clone();

    if (mDebugImage.type() == CV_8UC1) {
        cv::cvtColor(mDebugImage, mDebugImage, cv::COLOR_GRAY2RGB);
    }
    if (mDebugImage.type() == CV_8UC3) {
        cv::cvtColor(mDebugImage, mDebugImage, cv::COLOR_RGB2BGR);
    }
    if (mDebugImage.type() == CV_8UC4) {
        cv::cvtColor(mDebugImage, mDebugImage, cv::COLOR_RGBA2BGR);
    }

    QImage qimage(mDebugImage.data, image.cols, image.rows, image.cols * 3, QImage::Format_RGB888);

    mPixmapDebug->setPixmap(QPixmap::fromImage(qimage));
    mPixmapDebug->setPos(0, 0);
    ui->graphicsView_debug->fitInView(0, 0, image.cols, image.rows, Qt::AspectRatioMode::KeepAspectRatio);
}

// use this to sort the points by x-value
struct myclass {
    bool operator()(cv::Point pt1, cv::Point pt2) {
        return (pt1.x < pt2.x);
    }
} myobject;

// use this to sort the points by y-value
struct myclass2 {
    bool operator()(cv::Point pt1, cv::Point pt2) {
        return pt1.y < pt2.y;
    }
} myobject2;

void MainWindow::matchFields(cv::Mat debugImage, cv::Mat cameraImage) {

    // empty vector
    fields.clear();

    // Apply template Matching
    cv::Mat res_32f(debugImage.rows - templateHeight + 1, debugImage.cols - templateWidth + 1, CV_32FC1);
    cv::cvtColor(debugImage, debugImage, cv::COLOR_BGR2GRAY);

    cv::matchTemplate(debugImage, templateImage, res_32f, cv::TM_CCOEFF_NORMED);

    // Apply threshold
    cv::Mat res;

    res_32f.convertTo(res, CV_8U, 255.0);
    cv::adaptiveThreshold(res, res, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, templateSize, -128);
    // draw rectangle around matches and mark current match as drawn
    while (true) {
        double minval, maxval;
        cv::Point minloc, maxloc;
        cv::minMaxLoc(res, &minval, &maxval, &minloc, &maxloc);

        if (maxval > 0) {
            cv::rectangle(cameraImage, cv::Rect(maxloc.x, maxloc.y, templateWidth, templateHeight), (0, 255, 0), 5);
            cv::floodFill(res, maxloc, 0); //mark drawn blob, important!
            // add matches to vector
            fields.push_back(cv::Point(maxloc.x + coinRadius, maxloc.y + coinRadius));
            cv::circle(cameraImage, cv::Point(maxloc.x + coinRadius, maxloc.y + coinRadius), 40, (255, 255, 125), 4);
        } else
            break;
    }
    // sort with inaccuracy
    std::sort(fields.begin(), fields.end(), myobject2);
    for (int i = 0; i < 6; ++i) {
        std::sort(fields.begin() + i * 7, fields.begin() + i * 7 + 7, myobject);
    }
}

void MainWindow::colorDetection(cv::Mat image) {
    // convert image to hsv for better color-detection
    cv::Mat img_hsv, maskR, maskY, mask1, mask2;
    cv::cvtColor(image, img_hsv, cv::COLOR_BGR2HSV);

    // Gen lower mask (0-5) and upper mask (175-180) of RED
    cv::inRange(img_hsv, cv::Scalar(0, 50, 20), cv::Scalar(5, 255, 255), mask1);
    cv::inRange(img_hsv, cv::Scalar(175, 50, 20), cv::Scalar(180, 255, 255), mask2);
    // Merge the masks
    cv::bitwise_or(mask1, mask2, maskR);
    // show red pixels

    // HUE for YELLOW is 21-30.
    int lowH = 21;
    int highH = 30;
    // Saturation
    int lowS = 190;
    int highS = 255;
    // Value
    int lowV = 20;
    int highV = 255;
    // Adjust Saturation and Value depending on the lighting condition of the environment
    cv::inRange(img_hsv, cv::Scalar(lowH, lowS, lowV), cv::Scalar(highH, highS, highV), maskY);
    // show yellow pixels

    // TODO: field needs to have 6*7 entries to set the coins correctly
    // std::cout << fields.size() << std::endl;

    // fill 2d-vector with coins
    int position;
    // std::cout << fields << std::endl;
    coins.resize(6, std::vector<int>(7, 0));
    redCoins = 0;
    yellowCoins = 0;
    for (int j = 5; j >= 0; j--) {
        for (int i = 6; i >= 0; i--) {
            position = j*7+i;
            // std::cout<< position << std::endl;
            // std::cout << "position: " << position << std::endl;
            // std::cout << "i,j: " << i << ", " << j << std::endl;
            // std::cout << "maskR.at(" << position << ")=" << maskR.at<float>(fields[position].x, fields[position].y) << std::endl;
            // cv::circle(maskR, cv::Point(fields[i].x, fields[i].y), 40, 125, 4);

            // red coins
            if (maskR.at<double>(fields[position].x, fields[position].y) == 255) {
                // this pixel is white in mask -> red on the src-image
                if (j < 5 && coins[j + 1][i] != 0) {
                    // check if field underneath (j+1) has already a coin
                    coins[j][i] = 1;
                    std::cout << "coin red set" << std::endl;
                    redCoins++;
                } else if (j == 5) {
                    // check if j the lowest level
                    coins[j][i] = 1;
                    std::cout << "coin red set" << std::endl;
                    redCoins++;
                }
            }

            // yellow coins
            if (maskY.at<double>(fields[position].x, fields[position].y) == 255) {
                // this pixel is white in mask -> yellow on the src-image
                if (j < 5 && coins[j + 1][i] != 0) {
                    // check if field underneath (j+1) has already a coin
                    coins[j][i] = 2;
                    std::cout << "coin yellow set" << std::endl;
                    yellowCoins++;
                } else if (j == 5) {
                    // check if j the lowest level
                    coins[j][i] = 2;
                    std::cout << "coin yellow set" << std::endl;
                    yellowCoins++;
                }
            }
        }
    }

    // print values of mask-Red at fields-coordinates
    for (int i = 0; i < 42; ++i) {
        std::cout << maskR.at<int>(fields[i].x, fields[i].y) << ' ';
        if(i%7==0){
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

    /*
    // print fields 2d-vector
    std::cout <<fields<< std::endl;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 7; ++j) {
            std::cout << fields[i*6+j].x << "," << fields[i*6+j].y << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    */
    /*
    // print coins vector
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 7; ++j) {
            std::cout << coins[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    */
/*
    // draw index of fields onto mask
    for (int i = 0; i < fields.size(); ++i) {
        std::ostringstream convert;
        convert << i;
        cv::putText(maskR, convert.str(), cv::Point(fields[i].x, fields[i].y), cv::FONT_HERSHEY_DUPLEX, 1.0, 125);
        // cv::circle(maskR, cv::Point(fields[i].x, fields[i].y), 40, 125, 4);
    }
    // cv::imshow("yellowMask", maskY);
    // cv::imshow("redMask", maskR);
*/
    //cv::imshow("redMask", maskR);

    ui->label_yellow->setText(QString::number(yellowCoins) + " coins set");
    ui->label_red->setText(QString::number(redCoins) + " coins set");
}

// return 0 if no win, 1 if red won, 2 if yellow won
int MainWindow::checkWin() {
    int boardHeight = 6;
    int boardWidth = 7;

    // check horizontal spaces
    for (int y = 0; y < boardHeight; ++y) {
        for (int x = 0; x < boardWidth - 3; ++x) {
            if (coins[x][y] == coins[x + 1][y] == coins[x + 2][y] == coins[x + 3][y]) {
                if (coins[x][y] == 1 || coins[x][y] == 2) {
                    return coins[x][y];
                }
            }
        }
    }
    // check vertical spaces
    for (int x = 0; x < boardWidth; ++x) {
        for (int y = 0; y < boardHeight - 3; ++y) {
            if (coins[x][y] == coins[x][y + 1] == coins[x][y + 2] == coins[x][y + 3]) {
                if (coins[x][y] == 1 || coins[x][y] == 2) {
                    return coins[x][y];
                }
            }
        }
    }
    // check / diagonal spaces
    for (int x = 0; x < boardWidth - 3; ++x) {
        for (int y = 3; y < boardHeight; y++) {
            if (coins[x][y] == coins[x + 1][y - 1] == coins[x + 2][y - 2] == coins[x + 3][y - 3]) {
                if (coins[x][y] == 1 || coins[x][y] == 2) {
                    return coins[x][y];
                }
            }
        }
    }
    // check \ diagonal spaces
    for (int x = 0; x < boardWidth - 3; ++x) {
        for (int y = 0; y < boardHeight - 3; ++y) {
            if (coins[x][y] == coins[x + 1][y + 1] == coins[x + 2][y + 2] == coins[x + 3][y + 3]) {
                if (coins[x][y] == 1 || coins[x][y] == 2) {
                    return coins[x][y];
                }
            }
        }
    }
    return 0;
}

void MainWindow::processSingleFrame() {
    QElapsedTimer measureTime;
    measureTime.start();

    cv::Mat cameraImage;

    mCameraStream >> cameraImage;

    this->setCameraImage(cameraImage);

    // hier müssen Sie Ihren code einbauen

    cv::Mat debugImage = cameraImage.clone();

    // bearbeiten Sie das debug bild wie sie wollen;

    // std::cout << fields << std::endl;

    // TODO: set matchFields() as calibration function and not for every frame
    if (arraySet == 0) {

        matchFields(cameraImage, cameraImage);
        arraySet = 1;
    }

    colorDetection(cameraImage); // stack smashing

    this->setDebugImage(cameraImage);
    // sie können auch rechtecke oder linien direkt ins bild reinmalden

    // mSceneCamera.addItem(QGraphicsRectItem(...));

    // darunter sollte nichts geändert werden

    qint64 elapsedTime = measureTime.elapsed();

    ui->label_processingTime->setText(QString::number(elapsedTime) + " ms");
}


void MainWindow::setLoopTime(int value) {
    mTimer.setInterval(value);
    std::cout << "Setting new interval '" << value << "'" << std::endl;
}

void MainWindow::start() {
    mTimer.setInterval(ui->spinBox_loop->value());
    mTimer.start();
    std::cout << "Start" << std::endl;
    ui->label_status->setText("Status: Running");
}

void MainWindow::stop() {
    mTimer.stop();
    std::cout << "Stop" << std::endl;
    ui->label_status->setText("Status: Stopped");
}

void MainWindow::step() {
    mTimer.stop();
    mTimer.singleShot(0, this, SLOT(processSingleFrame()));
    std::cout << "Single Step" << std::endl;
    ui->label_status->setText("Status: Single Step");
}
