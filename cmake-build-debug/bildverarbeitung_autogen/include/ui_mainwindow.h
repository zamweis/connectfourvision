/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_3;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QGraphicsView *graphicsView_debug;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_stop;
    QPushButton *pushButton_start;
    QPushButton *pushButton_step;
    QLabel *label_status;
    QLabel *label_3;
    QSpinBox *spinBox_loop;
    QLabel *label;
    QLabel *label_processingTime;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QGraphicsView *graphicsView_camera;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_red;
    QLabel *label_4;
    QPushButton *pushButton_reset;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(849, 535);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout_3 = new QGridLayout(centralwidget);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        groupBox_2 = new QGroupBox(centralwidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        graphicsView_debug = new QGraphicsView(groupBox_2);
        graphicsView_debug->setObjectName(QString::fromUtf8("graphicsView_debug"));

        gridLayout_2->addWidget(graphicsView_debug, 0, 0, 1, 1);


        gridLayout_3->addWidget(groupBox_2, 0, 1, 1, 1);

        groupBox_3 = new QGroupBox(centralwidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_3);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_stop = new QPushButton(groupBox_3);
        pushButton_stop->setObjectName(QString::fromUtf8("pushButton_stop"));

        horizontalLayout->addWidget(pushButton_stop);

        pushButton_start = new QPushButton(groupBox_3);
        pushButton_start->setObjectName(QString::fromUtf8("pushButton_start"));

        horizontalLayout->addWidget(pushButton_start);

        pushButton_step = new QPushButton(groupBox_3);
        pushButton_step->setObjectName(QString::fromUtf8("pushButton_step"));

        horizontalLayout->addWidget(pushButton_step);

        label_status = new QLabel(groupBox_3);
        label_status->setObjectName(QString::fromUtf8("label_status"));

        horizontalLayout->addWidget(label_status);

        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout->addWidget(label_3);

        spinBox_loop = new QSpinBox(groupBox_3);
        spinBox_loop->setObjectName(QString::fromUtf8("spinBox_loop"));
        spinBox_loop->setMaximum(10000);
        spinBox_loop->setValue(100);

        horizontalLayout->addWidget(spinBox_loop);

        label = new QLabel(groupBox_3);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        label_processingTime = new QLabel(groupBox_3);
        label_processingTime->setObjectName(QString::fromUtf8("label_processingTime"));
        label_processingTime->setMinimumSize(QSize(80, 0));
        label_processingTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_processingTime);


        horizontalLayout_2->addLayout(horizontalLayout);


        gridLayout_3->addWidget(groupBox_3, 1, 0, 1, 2);

        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        graphicsView_camera = new QGraphicsView(groupBox);
        graphicsView_camera->setObjectName(QString::fromUtf8("graphicsView_camera"));

        gridLayout->addWidget(graphicsView_camera, 0, 0, 1, 1);


        gridLayout_3->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_4 = new QGroupBox(centralwidget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        horizontalLayout_3 = new QHBoxLayout(groupBox_4);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_red = new QLabel(groupBox_4);
        label_red->setObjectName(QString::fromUtf8("label_red"));

        horizontalLayout_3->addWidget(label_red);

        label_4 = new QLabel(groupBox_4);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_3->addWidget(label_4);

        pushButton_reset = new QPushButton(groupBox_4);
        pushButton_reset->setObjectName(QString::fromUtf8("pushButton_reset"));

        horizontalLayout_3->addWidget(pushButton_reset);


        gridLayout_3->addWidget(groupBox_4, 2, 0, 1, 2);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 849, 19));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MainWindow", "Result or Debug View", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("MainWindow", "Controls", nullptr));
        pushButton_stop->setText(QCoreApplication::translate("MainWindow", "Stop", nullptr));
        pushButton_start->setText(QCoreApplication::translate("MainWindow", "Start", nullptr));
        pushButton_step->setText(QCoreApplication::translate("MainWindow", "Single Step", nullptr));
        label_status->setText(QCoreApplication::translate("MainWindow", "Status: stopped", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Loop time:", nullptr));
        spinBox_loop->setSuffix(QCoreApplication::translate("MainWindow", " ms", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Processing time:", nullptr));
        label_processingTime->setText(QCoreApplication::translate("MainWindow", "ms", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Camera Stream", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("MainWindow", "Game Stats", nullptr));
        label_red->setText(QCoreApplication::translate("MainWindow", "red won 0 rounds", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "yellow won 0 rounds", nullptr));
        pushButton_reset->setText(QCoreApplication::translate("MainWindow", "Reset Game", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
