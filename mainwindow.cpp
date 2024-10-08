#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>
#include <QImage>
#include <QPixmap>
#include <opencv2/core/types.hpp>


std::vector<cv::Point> fields;
std::vector<std::vector<int>> coins;
std::vector<cv::Vec3f> circles;
int redCoins = 0;
int yellowCoins = 0;
int yellowRounds = 0;
int redRounds = 0;
bool roundWon = false;
bool fieldsDetected = false;
bool roundEnd = false;

cv::Mat maskR, maskY;

// load templateImage
cv::Mat templateImage = cv::imread("template.png");
int fieldWidth;

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

    // wenn sie mehrere Kameras haben müssen Sie hier die Kamera mit einem anderen index wählen
    // mCameraStream.open(1);
    mCameraStream = cv::VideoCapture("testvideo.mp4");
    start();
    calibrate();
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

void MainWindow::detectFields(cv::Mat image) {

    // empty vector
    fields.clear();
    circles.clear();

    cv::Mat gray;
    cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 5);

    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, 25,  // change this value to detect circles with different distances to each other
                     20, 25, 15, 50 // change the last two parameters to detect larger/smaller circles
    );
    int avgRadius = 0;
    // calculate average radius for better circle detection
    for (size_t i = 0; i < circles.size() && i < 42; i++) {
        cv::Vec3i c = circles[i];
        avgRadius += c[2];
    }

    // find circles with avg radius
    avgRadius = avgRadius / 42;
    circles.clear();
    fieldWidth = 0;
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1,
                     avgRadius + 5,  // change this value to detect circles with different distances to each other
                     20, 25, avgRadius - 5, avgRadius + 5 // change the last two parameters to detect larger/smaller circles
    );
    for (size_t i = 0; i < circles.size() && i < 42; i++) {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]);
        // draw circle
        int radius = c[2];
        fieldWidth += radius;
        cv::circle(image, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
        fields.push_back(center);
    }
    fieldWidth = fieldWidth / 21 + 4; // draw around fields
    // check if all fields were found
    if (fields.size() == 42) {
        // sort and respect inaccuracy
        std::sort(fields.begin(), fields.end(), sortFuncY);
        for (int i = 0; i < 6; ++i) {
            std::sort(fields.begin() + i * 7, fields.begin() + i * 7 + 7, sortFuncX);
        }
        fieldsDetected = true;
    } else {
        fieldsDetected = false;
    }
}

void MainWindow::colorDetection(cv::Mat image) {
    // convert image to hsv for better color-detection
    cv::Mat img_hsv, mask1, mask2;
    cv::cvtColor(image, img_hsv, cv::COLOR_BGR2HSV);

    // Gen lower mask (0-5) and upper mask (175-180) of RED
    cv::inRange(img_hsv, cv::Scalar(0, 20, 20), cv::Scalar(5, 255, 255), mask1);
    cv::inRange(img_hsv, cv::Scalar(175, 20, 20), cv::Scalar(180, 255, 255), mask2);
    // Merge the masks
    cv::bitwise_or(mask1, mask2, maskR);

    // HUE for YELLOW is 21-30.
    // Adjust Saturation and Value depending on the lighting condition of the environment
    cv::inRange(img_hsv, cv::Scalar(10, 0, 0), cv::Scalar(35, 255, 255), maskY);

    // show masks
    //cv::imshow("yellowMask", maskY);
    //cv::imshow("redMask", maskR);
}

void MainWindow::insertCoins(cv::Mat image) {
    // check if field were all found
    if (fieldsDetected) {

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
                        cv::rectangle(image,
                                      cv::Rect(fields[position].x - fieldWidth / 2, fields[position].y - fieldWidth / 2,
                                               fieldWidth, fieldWidth), CV_RGB(255, 0, 0), 2);

                    } else if (j < 5 && coins[j + 1][i] != 0) {
                        // check if field underneath (j+1) has already a coin
                        coins[j][i] = 1;
                        redCoins++;
                        cv::rectangle(image,
                                      cv::Rect(fields[position].x - fieldWidth / 2, fields[position].y - fieldWidth / 2,
                                               fieldWidth, fieldWidth), CV_RGB(255, 0, 0), 2);
                    }
                }

                // yellow coins
                if (maskY.at<uchar>(fields[position]) == 255 && coins[j][i] == 0) {
                    // this pixel is white in mask -> yellow on the src-image
                    if (j == 5) {
                        // check if j the lowest level
                        coins[j][i] = 2;
                        yellowCoins++;
                        cv::rectangle(image,
                                      cv::Rect(fields[position].x - fieldWidth / 2, fields[position].y - fieldWidth / 2,
                                               fieldWidth, fieldWidth), CV_RGB(255, 255, 0), 2);
                    } else if (j < 5 && coins[j + 1][i] && coins[j][i] == 0) {
                        // check if field underneath (j+1) has already a coin
                        coins[j][i] = 2;
                        yellowCoins++;
                        cv::rectangle(image,
                                      cv::Rect(fields[position].x - fieldWidth / 2, fields[position].y - fieldWidth / 2,
                                               fieldWidth, fieldWidth), CV_RGB(255, 255, 0), 2);
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
}

// return 0 if no win, 1 if red won, 2 if yellow won
int MainWindow::checkWin(cv::Mat image) {
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
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y), CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
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
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y), CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
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
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x, fields[fieldPosEnd].y),
                             CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
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
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x,
                                       fields[fieldPosEnd].y), CV_RGB(255, 0, 0), 3);
                    winner = 1;
                } else if (coins[y][x] == 2) {
                    cv::line(image, cv::Point(fields[fieldPosStart].x, fields[fieldPosStart].y),
                             cv::Point(fields[fieldPosEnd].x,
                                       fields[fieldPosEnd].y), CV_RGB(255, 255, 0), 3);
                    winner = 2;
                }
            }
        }
    }
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

    // check if fields are calibrated
    if (fieldsDetected) {

        // check if round has been won
        if (!roundWon) {

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
            for (size_t i = 0; i < circles.size() && i < 42; i++) {
                cv::Vec3i c = circles[i];
                cv::Point center = cv::Point(c[0], c[1]);

                // draw circle
                int radius = c[2];
                std::ostringstream convert;
                convert << i;
                cv::circle(cameraImage, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
                cv::putText(cameraImage, convert.str(), cv::Point(fields[i].x - 10, fields[i].y + 5), cv::FONT_HERSHEY_DUPLEX, 0.6,
                            cv::Scalar(255, 0, 255));
                fields.push_back(center);
            }
            this->setDebugImage(cameraImage);

            // uncomment this to prevent stop when a team wins
            roundWon = false;

        } else {
            if (!roundEnd) {
                // check if still win for next 50 frames
                int winner;
                for (int i = 0; i < 50; ++i) {
                    //get 10 more frames
                    mCameraStream >> cameraImage;
                }
                // detect coins
                colorDetection(cameraImage);

                // insert coins into 2d vector
                insertCoins(cameraImage);
                winner = checkWin(cameraImage);
                for (size_t i = 0; i < circles.size() && i < 42; i++) {
                    cv::Vec3i c = circles[i];
                    cv::Point center = cv::Point(c[0], c[1]);
                    // draw circle
                    int radius = c[2];
                    std::ostringstream convert;
                    convert << i;
                    cv::circle(cameraImage, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
                    cv::putText(cameraImage, convert.str(), cv::Point(fields[i].x - 10, fields[i].y + 5), cv::FONT_HERSHEY_DUPLEX, 0.6,
                                cv::Scalar(255, 0, 255));
                    fields.push_back(center);
                }
                this->setDebugImage(cameraImage);
                if (winner != 0) {
                    roundWon = true;
                    roundEnd = true;
                    std::cout << "WINNRE WINNER, CHICKEN DINNER" << std::endl;
                    std::cout << "Empty field for next round, calibrate and press start" << std::endl;
                    ui->label_status->setText("Status: Round Over");
                } else {
                    roundWon = false;
                }
            }
        }
    } else {
        this->setDebugImage(cameraImage);
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
    roundEnd = false;
    roundWon = false;
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
    ui->label_status->setText("Status: Calibrate");
    cv::Mat cameraImage;
    mCameraStream >> cameraImage;

    if (fieldsDetected) {
        // check if no coins are set
        colorDetection(cameraImage);
        insertCoins(cameraImage);
        if (std::all_of(coins.begin(), coins.end(), [](const std::vector<int> &v) {
            return std::all_of(v.begin(), v.end(), [](int x) { return x == 0; });
        })) {
            detectFields(cameraImage);
            std::cout << "Calibrate Success" << std::endl;
        } else {
            std::cout << "Please remove coins before calibrating" << std::endl;
            std::cout << "Calibrate Failed" << std::endl;
        }
    } else {
        detectFields(cameraImage);
        if (fields.size() != 42) {
            std::cout << "Not all fields found: " << fields.size() << std::endl;
            std::cout << "Calibrate Failed" << std::endl;
        } else {
            std::cout << "Calibrate Success" << std::endl;
        }
    }
}
