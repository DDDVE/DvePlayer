#include <QWidgetAction>
#include <QCheckBox>
#include <QtDebug>
#include <QFileDialog>
#include <algorithm>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "log.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QMenu *fileMenu = new QMenu(this);
    fileMenu->addAction(ui->actionOpen);
    fileMenu->addAction(ui->actionSave);
    ui->toolButton->setMenu(fileMenu);

    // set click action
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onClickOpenFile);
    // set click pause button
    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::onReleasePauseButton);

    // set volume change
    connect(ui->verticalSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);

    mPlayer = new Player(*this);
    // set resize action
    connect(mPlayer->mVideoPlayer, &VideoPlayer::sigResizeWindows, this, &MainWindow::SetPlayerSize);

     // set progress bar change
    connect(ui->horizontalSlider, &QSlider::sliderPressed, this, &MainWindow::onProgressPress);
    connect(ui->horizontalSlider, &QSlider::valueChanged, mPlayer, &Player::onChangePlayProgress);
    connect(ui->horizontalSlider, &QSlider::sliderReleased, this, &MainWindow::onProgressRelease);

    // set play speed change
    connect(ui->horizontalSlider_2, &QSlider::sliderPressed, this, &MainWindow::onPlaySpeedPress);
    connect(ui->horizontalSlider_2, &QSlider::valueChanged, mPlayer, &Player::onChangePlaySpeed);
    connect(ui->horizontalSlider_2, &QSlider::sliderReleased, this, &MainWindow::onPlaySpeedRelease);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPlaySpeedPress()
{
    mPlayer->mIsChangingSpeed = true;
}

void MainWindow::onPlaySpeedRelease()
{
    emit ui->horizontalSlider_2->valueChanged(-10);
}

void MainWindow::onProgressPress()
{
    mPlayer->mIsChangingProgress = true;
}

void MainWindow::onProgressRelease()
{
    emit ui->horizontalSlider->valueChanged(-10);
}

void MainWindow::onVolumeChanged(int val)
{
    mPlayer->mAudioVolume = val / 100.0;
    if (!mPlayer->mAudioPlayer->isRunning())
    {
        return;
    }
    if (!mPlayer->mAudioPlayer->mAudioOutput)
    {
        return;
    }
    mPlayer->mAudioPlayer->mAudioOutput->setVolume(val / 100.0);
//    qDebug() << "[INFO] Set volume=[" << val / 100.0 << "]." << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
}

void MainWindow::onReleasePauseButton()
{
    // check whether is playing
    if (!mPlayer->mVideoPlayer->isRunning() && !mPlayer->mAudioPlayer->isRunning())
    {
        return;
    }
    mPlayer->mIsPause = (mPlayer->mIsPause)? false : true;
}

void MainWindow::onClickOpenFile()
{
    qDebug() << "[INFO] click open file." << LOG_FUNCTION_AND_LINE
             << getMiliSecondTimeStamp();
    QString file = QFileDialog::getOpenFileName(nullptr);
    qDebug() << "[INFO] choose file=[" << file << "]."
             << LOG_FUNCTION_AND_LINE
             << getMiliSecondTimeStamp();
    if (file.isEmpty())
    {
        qDebug() << "[INFO] choose no file."
                 << LOG_FUNCTION_AND_LINE
                 << getMiliSecondTimeStamp();
        return;
    }
    // inform player object
    mPlayer->Start(file);
    qDebug() << LOG_LEAVE_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
}

/* resize window by image size */
void MainWindow::SetPlayerSize(int width, int height)
{
    qDebug() << "[INFO] Set main window width = [" << width << "], height = ["
             << height << "]."
             << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    this->resize(width + ui->verticalLayoutWidget->width() + 10,
                 height + ui->horizontalLayoutWidget->height() + 10);
    // change bottom playing layout
    ui->horizontalLayoutWidget_2->setGeometry(ui->verticalLayoutWidget->width(),
                                              this->height() - ui->horizontalLayoutWidget_2->height(),
                                              ui->horizontalLayoutWidget_2->width(),
                                              ui->horizontalLayoutWidget_2->height());
}

void MainWindow::paintEvent(QPaintEvent *event)
{
//    qDebug() << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    QPainter tmpPainter(this);
    gImgMut.lock();
    tmpPainter.drawPixmap(QRect(ui->verticalLayoutWidget->x() + ui->verticalLayoutWidget->width(),
                                ui->verticalLayoutWidget->y(), mPlayer->mPixmap.width(), mPlayer->mPixmap.height()),
                                mPlayer->mPixmap);
    gImgMut.unlock();

    // change bottom playing layout
    ui->horizontalLayoutWidget_2->setGeometry(ui->verticalLayoutWidget->width() + 10,
                                              this->height() - ui->horizontalLayoutWidget_2->height()-ui->pushButton->height(),
                                              ui->horizontalLayoutWidget_2->width(),
                                              ui->horizontalLayoutWidget_2->height());
    // draw volume
    double tmpVolume = mPlayer->mAudioVolume * 100;
    ui->verticalSlider->setValue(tmpVolume);

    // draw paly progress
    if (!mPlayer->mIsChangingProgress)
    {
        ui->horizontalSlider->setValue(mPlayer->mPlayProgressRate * 100);
    }

    // draw play speed
    if (!mPlayer->mIsChangingSpeed)
    {
        ui->horizontalSlider_2->setValue(mPlayer->mPlaySpeed * 50);
    }

    // draw pause button
    if (!mPlayer->mIsPause)
    {
        ui->pushButton->setText("Pause");
    }
    else
    {
        ui->pushButton->setText("Start");
    }

//    qDebug() << LOG_LEAVE_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
}

