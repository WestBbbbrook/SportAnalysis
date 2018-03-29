#ifndef REPLAYMDTHREAD_H
#define REPLAYMDTHREAD_H
#include "readfilethread.h"
#include "replaythread.h"
#include "quanju.h"

extern int turn;
extern int turnMD;
extern bool replayNextMedia;
extern bool nowPlayingMD;
extern int maxindex;

using namespace cv;
using namespace std;
class ReplayMDThread:public QThread
{
    Q_OBJECT
public:
    ReplayMDThread();
    virtual void run();
    void qsleep(unsigned int msec);

    void drawline(Mat &img,Point a, Point b,Scalar color);
    QImage cvMat2QImage(const cv::Mat& mat);

    Mat image_md;
    QImage img_md;
signals:     //这里制造一个名为Log的信号
    void replayMD(QImage  image_md);//给主线程的信号，回放

};

#endif // REPLAYMDTHREAD_H
