#ifndef READFILETHREAD_H
#define READFILETHREAD_H
#include  "quanju.h"

#include <cstdlib>
#include <QDir>
#include <iostream>
#include<io.h>
#include<fstream>
#include <QThread>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<vector>
#include<string>
#define X_CHANGE_MD 400
#define Y_CHANGE_MD 150
#define W_MD 30.0
using namespace cv;
using namespace std;
class readFileThread : public QThread
{
    Q_OBJECT
public:
    readFileThread();
    virtual void run();
public:
    vector<vector<Point> > readDBPoints(string txtfilename);//把单个模版视频里的每一帧关键点信息放进vector
    void getFiles( string path, vector<string>& files );//获取一个文件夹中所有文件的路径，获取每个模版视频路径
    void SplitString(const string &s, vector<string> &v, const string &c);//解析服务端发来的字符串：bbox、关键点、分数

};

#endif // READFILETHREAD_H

