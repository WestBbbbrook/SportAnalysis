#ifndef MYTHREAD_H
#define MYTHREAD_H
#include "winsockmattransmissionclient.h"

#include <QString>
#include <QThread>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace cv;
class Mythread : public QThread
{
    Q_OBJECT
public:
    Mythread();
    void SplitString(const string &s, vector<string> &v, const string &c);//拆分收到的字符串
    int  ExplainString(string str);//解析收到的字符串
public:
       virtual void run();
signals:     //这里制造一个名为Log的信号
        void replay(QString   sMessage);//给主线程的信号，回放
        void play2(QString   sMessage);//给主线程的信号，画bbox或点，在界面二显示
};
#endif // MYTHREAD_H
