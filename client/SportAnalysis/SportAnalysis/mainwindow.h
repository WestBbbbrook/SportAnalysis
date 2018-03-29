#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "mythread.h"
#include "winsockmattransmissionclient.h"
#include "mythreadcoordinate.h"
#include "readfilethread.h"

#include "replaythread.h"
#include "replaymdthread.h"
#include "quanju.h"

#include <QImage>
#include <iostream>
#include <QTimer>
#include <QTime>

#include <QMainWindow>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<QString>
#include<QLabel>
#include<QVector>
#include <string>
#include<io.h>
#include<fstream>

#include <QMouseEvent>
#define LOCALIP "10.103.248.103"	// 本机IP，测试用
#define SERVERIP "10.103.240.172"	// 服务器IP
//#define SERVERIP "192.168.1.102"   // 服务器IP
#define TRANS_WIDTH 656//560	// 传输的MAT宽度
#define TRANS_HEIGHT 368//315 // 传输的MAT高度

#define GME_Threshold 1.92 //GME阈值
#define VIBE_Threshold 10//vibe阈值
extern float w_label_9;

extern vector<vector<Point>> mediaPoints;//所有帧人的关键点集合，两层vector：帧，点
extern vector<double> scores;//每个动作的分数，
extern WinsockMatTransmissionClient socketMat;//传输图像，bbox，关键点、分数的socket
extern WinsockMatTransmissionClient socketCoordinate;//发送点击鼠标位置的socket
extern bool beginStoreMedia;//收到关键点，开始存mat
extern bool drawBbox;//绘制bbox
extern bool replayNextMedia;//重放下一段？
extern bool nowPlaying;//正在重放？
extern bool nowPlayingMD;//正在重放？

extern int maxindex;//分数最大的视频的索引，重放模板视频关键点用

extern vector<Rect> boundingboxs;//每一帧的boundingbox集合
extern vector<Point> points;//录制时每一帧人的关键点集合
extern vector<Point> points1;//不录制的时候每一帧人的关键点集合

extern int mouseX;//鼠标点击的x
extern int mouseY;//鼠标点击的y

extern int label9width;
extern int label9height;


using namespace std;
using namespace cv;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void replayStart(QString sMessage);//接收来自子线程mythread的emit信号，开始回放视频
    void play2(QString sMessage);//接收来自子线程mythread的emit信号，开始在第二个界面显示带关键点或者bbox的视频
    void nextCamereFrame();//读取下一摄像头文件帧
    void replayC(QImage   image);//给主线程的信号，回放
    void replayMD(QImage   image);//给主线程的信号，回放

public:
    void qsleep(unsigned int msec);//延迟函数
    void drawline(Mat &img,Point a, Point b,Scalar color);//画线
    QImage cvMat2QImage(const cv::Mat& mat);//Mat转换成QImage
    float xy_change_md();
    Mat drawScore(bool drawscore);


    Mat& MyGammaCorrection(Mat& I, float fGamma);
    bool cvMatEQ(const cv::Mat& data1, const cv::Mat& data2);
    double count_GM(Mat oriFrame);
    int vibe(Mat frame_now);
    Mat MoveDetect(Mat frame1, Mat frame2);
protected:
    void mousePressEvent(QMouseEvent *e); //鼠标点击事件
private:
    Ui::MainWindow *ui;
    Mythread receriveThread;//实时接收数据线程
    readFileThread readFileThread;//读取本地模板视频关键点线程
    MythreadCoordinate ThreadCoordinate;//发送鼠标点击坐标线程

    VideoCapture captureCamera;
    cv::Mat cframe;//摄像头MAT，第一个界面cameraframe
    cv::Mat mframe;//截取的摄像头MAT，第二个界面mediaframe

    QTimer *timer1;//延迟播放视频用

    Size dsize;//socket传送的mat数据大小
    Mat playframe;//下一次要显示的视频帧
    Mat nowframe;//当前视频帧

    int framenum;//帧序号，跳帧用
    ReplayThread rt;
    ReplayMDThread rmdt;
};

#endif // MAINWINDOW_H
