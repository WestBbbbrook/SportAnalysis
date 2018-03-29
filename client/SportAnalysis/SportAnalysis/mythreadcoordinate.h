#ifndef MYTHREADCOORDINATE_H
#define MYTHREADCOORDINATE_H
#include "winsockmattransmissionclient.h"

#include <QString>
#include <QThread>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>
using namespace std;
using namespace cv;
class MythreadCoordinate : public QThread
{
    Q_OBJECT
public:
    MythreadCoordinate();
public:
       virtual void run();//向服务端发点击坐标
signals:     //这里制造一个名为Log的信号
        void Log(QString   sMessage);

};
#endif // MYTHREADCOORDINATE_H
