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
int yellowRounds = 0;
int redRounds = 0;
bool roundWon = false;

cv::Mat maskR, maskY;

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
    connect(ui->pushButton_reset, SIGNAL(clicked()), this, SLOT(reset()));
    connect(ui->pushButton_calibrate, SIGNAL(clicked()), this, SLOT(calibrate()));


    ui->graphicsView_camera->setScene(&mSceneCamera);
    ui->graphicsView_debug->setScene(&mSceneDebug);

    mPixmapCamera = mSceneCamera.addPixmap(QPixmap());
    mPixmapDebug = mSceneDebug.addPixmap(QPixmap());

    //wenn sie mehrere Kameras haben müssen Sie hier die Kamera mit einem anderen index wählen
    //mCameraStream.open(1);
    mCameraStream = cv::VideoCapture("testvideo2.mp4");
    start();

}


MainWindow::~MainWindow() {
    delete ui;
}

// use this to sort the points by x-value
struct sortX {
    bool operator()(cv::Point pt1, cv::Point pt2) {
        return (pt1.x < pt2.x);
    }
} sortFuncX;

// use this to sort the points by y-value
struct sortY {
    bool operator()(cv::Point pt1, cv::Point pt2) {
        return pt1.y < pt2.y;
    }
} sortFuncY;

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
            // draw rectangle around fields for debug
            // cv::rectangle(cameraImage, cv::Rect(maxloc.x, maxloc.y, templateWidth, templateHeight), CV_RGB(0, 255, 0),);
            cv::floodFill(res, maxloc, 0); //mark drawn blob, important!

            // add matches to vector
            fields.push_back(cv::Point(maxloc.x + coinRadius, maxloc.y + coinRadius));
        } else
            break;
    }
    // sort and respect inaccuracy
    std::sort(fields.begin(), fields.end(), sortFuncY);
    for (int i = 0; i < 6; ++i) {
        std::sort(fields.begin() + i * 7, fields.begin() + i * 7 + 7, sortFuncX);
    }
}

void MainWindow::colorDetection(cv::Mat image) {
    // convert image to hsv for better color-detection
    cv::Mat img_hsv, mask1, mask2;
    cv::cvtColor(image, img_hsv, cv::COLOR_BGR2HSV);

    // Gen lower mask (0-5) and upper mask (175-180) of RED
    cv::inRange(img_hsv, cv::Scalar(0, 60, 60), cv::Scalar(5, 255, 255), mask1);
    cv::inRange(img_hsv, cv::Scalar(175, 60, 60), cv::Scalar(180, 255, 255), mask2);

    // Merge the masks
    cv::bitwise_or(mask1, mask2, maskR);

    // HUE for YELLOW is 21-30.
    // Adjust Saturation and Value depending on the lighting condition of the environment
    cv::inRange(img_hsv, cv::Scalar(0, 70, 70), cv::Scalar(50, 255, 255), maskY);

    /*
     * Debug for masks
     *
    // draw field numbers on masks for debug
    for (int i = 0; i < fields.size(); ++i) {
        std::ostringstream convert;
        convert << i;
        cv::putText(maskR, convert.str(), cv::Point(fields[i].x - 20, fields[i].y - 50), cv::FONT_HERSHEY_DUPLEX, 0.5,
                    125);
        cv::putText(maskY, convert.str(), cv::Point(fields[i].x - 20, fields[i].y - 50), cv::FONT_HERSHEY_DUPLEX, 0.5,
                    125);
    }
    // show masks
    cv::imshow("yellowMask", maskY);
    cv::imshow("redMask", maskR);
    */
}

void MainWindow::insertCoins(cv::Mat cameraImage) {
    // fill 2d-vector with coins
    int position = 41;
    coins.clear();
    coins.resize(6, std::vector<int>(7, 0));
    redCoins = 0;
    yellowCoins = 0;

    for (int j = 5; j >= 0; j--) {
        for (int i = 6; i >= 0; i--) {

            // red coins
            if (maskR.at<uchar>(fields[position]) == 255 && coins[j][i] == 0) {
                // this pixel is white in mask -> red on the src-image
                if (j == 5) {
                    // check if j the lowest level
                    coins[j][i] = 1;
                    redCoins++;
                    cv::rectangle(cameraImage,
                                  cv::Rect(fields[position].x - coinRadius, fields[position].y - coinRadius,
                                           templateWidth, templateHeight), CV_RGB(255, 0, 0), 5);

                } else if (j < 5 && coins[j + 1][i] != 0) {
                    // check if field underneath (j+1) has already a coin
                    coins[j][i] = 1;
                    redCoins++;
                    cv::rectangle(cameraImage,
                                  cv::Rect(fields[position].x - coinRadius, fields[position].y - coinRadius,
                                           templateWidth, templateHeight), CV_RGB(255, 0, 0), 5);
                }
            }

            // yellow coins
            if (maskY.at<uchar>(fields[position]) == 255 && coins[j][i] == 0) {
                // this pixel is white in mask -> yellow on the src-image
                if (j == 5) {
                    // check if j the lowest level
                    coins[j][i] = 2;
                    yellowCoins++;
                    cv::rectangle(cameraImage,
                                  cv::Rect(fields[position].x - coinRadius, fields[position].y - coinRadius,
                                           templateWidth, templateHeight), CV_RGB(255, 255, 0), 5);
                } else if (j < 5 && coins[j + 1][i] && coins[j][i] == 0) {
                    // check if field underneath (j+1) has already a coin
                    coins[j][i] = 2;
                    yellowCoins++;
                    cv::rectangle(cameraImage,
                                  cv::Rect(fields[position].x - coinRadius, fields[position].y - coinRadius,
                                           templateWidth, templateHeight), CV_RGB(255, 255, 0), 5);
                }
            }
            position--;
        }
    }
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

    ui->label_red->setText("Red Coins: " + QString::number(redCoins));
    ui->label_yellow->setText("Yellow Coins: " + QString::number(yellowCoins));
}

// return 0 if no win, 1 if red won, 2 if yellow won
int MainWindow::checkWin(cv::Mat cameraImage) {
    int boardHeight = 6;
    int boardWidth = 7;
    int winner = 0;
    int fieldPosStart;
    int fieldPosEnd;

    // check horizontal spaces
    for (int y = 0; y < boardHeight; y++) {
        for (int x = 0; x < boardWidth - 3; x++) {
            if ((coins[y][x] == coins[y][x + 1]) && (coins[y][x] == coins[y][x + 2]) &&
                (coins[y][x] == coins[y][x + 3])) {
                fieldPosStart = y * 7 + x;
                fieldPosEnd = y * 7 + x + 3;
                if (coins[y][x] == 1) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y), CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y), CV_RGB(255, 255, 0), 3);
                    winner = 2;
                }
            }
        }
    }

    // check vertical spaces
    for (int x = 0; x < boardWidth; x++) {
        for (int y = 0; y < boardHeight - 3; y++) {
            if ((coins[y][x] == coins[y + 1][x]) && (coins[y][x] == coins[y + 2][x]) &&
                (coins[y][x] == coins[y + 3][x])) {
                fieldPosStart = y * 7 + x;
                fieldPosEnd = (y + 3) * 7 + x;
                if (coins[y][x] == 1) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y), CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y), CV_RGB(255, 255, 0), 3);
                    winner = 2;
                }
            }
        }
    }

    // check / diagonal spaces
    for (int x = 0; x < boardWidth - 3; x++) {
        for (int y = 3; y < boardHeight; y++) {
            if ((coins[y][x] == coins[y - 1][x + 1]) && (coins[y][x] == coins[y - 2][x + 2]) &&
                (coins[y][x] == coins[y - 3][x + 3])) {
                fieldPosStart = y * 7 + x;
                fieldPosEnd = (y - 3) * 7 + x + 3;
                if (coins[y][x] == 1) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y),
                             CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y),
                             CV_RGB(255, 255, 0), 3);
                    winner = 2;
                }
            }
        }
    }

    // check \ diagonal spaces
    for (int x = 0; x < boardWidth - 3; x++) {
        for (int y = 0; y < boardHeight - 3; y++) {
            if ((coins[y][x] == coins[y + 1][x + 1]) && (coins[y][x] == coins[y + 2][x + 2]) &&
                (coins[y][x] == coins[y + 3][x + 3])) {
                fieldPosStart = y * 7 + x;
                fieldPosEnd = (y + 3) * 7 + x + 3;
                if (coins[y][x] == 1) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x,
                                       fields[fieldPosEnd].y), CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(cameraImage, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x,
                                       fields[fieldPosEnd].y), CV_RGB(255, 255, 0), 3);
                    winner = 2;
                }
            }
        }
    }
    if (winner != 0) std::cout << "WINNRE WINNER, CHICKEN DINNER" << std::endl;
    return winner;
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

void MainWindow::processSingleFrame() {
    QElapsedTimer measureTime;
    measureTime.start();

    cv::Mat cameraImage;

    mCameraStream >> cameraImage;

    this->setCameraImage(cameraImage);

    // check if round has been won
    if (!roundWon) {

        // check if fields are calibrated
        if (arraySet == 0) {
            matchFields(cameraImage, cameraImage);
            arraySet = 1;
        }

        // detect coins
        colorDetection(cameraImage);

        // insert coins into 2d vector
        insertCoins(cameraImage);
        
        // check for winner
        int winner = checkWin(cameraImage);
        if (winner == 1) {
            redRounds++;
            roundWon = true;
        } else if (winner == 2) {
            yellowRounds++;
            roundWon = true;
        }

        // uncomment this to prevent stop when a team wins
        // roundWon = false;

        this->setDebugImage(cameraImage);
    } else {
        stop();
        std::cout << "Empty field for next round and press start" << std::endl;
        ui->label_status->setText("Status: Round Won");
        roundWon = false;
    }

    ui->label_red_rounds->setText("Red Rounds: " + QString::number(redRounds));
    ui->label_yellow_rounds->setText("Yellow Rounds: " + QString::number(yellowRounds));


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

void MainWindow::reset() {
    mTimer.stop();
    std::cout << "Reset" << std::endl;
    // reset stats
    redCoins = 0;
    yellowCoins = 0;
    redRounds = 0;
    yellowRounds = 0;
    roundWon = false;
    coins.clear();
    // print stats
    ui->label_red->setText("Red Coins: " + QString::number(redCoins));
    ui->label_yellow->setText("Yellow Coins: " + QString::number(yellowCoins));
    ui->label_red_rounds->setText("Red Rounds: " + QString::number(redRounds));
    ui->label_yellow_rounds->setText("Yellow Rounds: " + QString::number(yellowRounds));
    ui->label_status->setText("Status: Reset");
}

void MainWindow::calibrate() {
    mTimer.stop();
    std::cout << "Calibrate" << std::endl;
    ui->label_status->setText("Status: Calibrate");
    cv::Mat cameraImage;
    mCameraStream >> cameraImage;

    // check if no coins are set
    colorDetection(cameraImage);
    insertCoins(cameraImage);
    if (std::all_of(coins.begin(), coins.end(), [](const std::vector<int> &v) {
        return std::all_of(v.begin(), v.end(), [](int x) { return x == 0; });
    })) {
        matchFields(cameraImage, cameraImage);
        std::cout << "Calibrate Success" << std::endl;
        start();
    } else {
        std::cout << "Please remove coins before calibrating" << std::endl;
        std::cout << "Calibrate Failed" << std::endl;
    }
}
