#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>
#include <QLayout>
#include <QPushButton>
#include <QPainter>

#include "player/player.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void onClickOpenFile();
    void SetPlayerSize(int width, int height);
    void onReleasePauseButton();
    void paintEvent(QPaintEvent *event);
    void onVolumeChanged(int val);


    Ui::MainWindow *ui;
    QToolButton *mFileBtn;
    QPushButton *mOpenFileBtn;
    Player *mPlayer;
    int mImgX;
    int mImgY;

Q_SIGNALS:
    void sigSetPlayerSize(int width, int height);


};
#endif // MAINWINDOW_H
