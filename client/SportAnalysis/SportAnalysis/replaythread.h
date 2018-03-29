#ifndef REPLAYTHREAD_H
#define REPLAYTHREAD_H
#include "quanju.h"

extern bool replayNextMedia;
extern bool nowPlaying;
using namespace cv;
using namespace std;
class ReplayThread:public QThread
{
    Q_OBJECT
public:
    ReplayThread();
    virtual void run();
    void qsleep(unsigned int msec);

    void drawline(Mat &img,Point a, Point b,Scalar color);
    QImage cvMat2QImage(const cv::Mat& mat);

    Mat image;
    QImage img;
signals:     //这里制造一个名为Log的信号
    void replayC(QImage  img);//给主线程的信号，回放
};
#endif // REPLAYTHREAD_H


