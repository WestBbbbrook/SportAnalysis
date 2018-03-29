#ifndef QUANJU_H
#define QUANJU_H
#include <QImage>
#include <iostream>
#include <QTimer>
#include <QTime>
#include <QThread>

#include <QMainWindow>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<QString>
#include<QLabel>
#include <string>
#include<io.h>
#include<fstream>
#include <vector>
#include <QCoreApplication>
#include <QEventLoop>
#include <iostream>
//extern int cbsize;

using namespace std;
using namespace cv;
extern vector<Mat> cameraBuffer;//存储视频
extern vector<Mat> cameraCopy;//回放时可能还在存储，所以先copy一下在重放
extern vector<vector<vector<Point> > >localAllPoint;

extern int repalyDelayMD;
class quanju
{
public:
    quanju();
};

#endif // QUANJU_H
