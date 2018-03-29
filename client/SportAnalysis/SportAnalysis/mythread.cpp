#include "mythread.h"
#include <QString>
#include <QVector>
#include <iostream>
using namespace std;
vector<vector<Point>> mediaPoints;
vector<double> scores;//每个动作的分数
vector<Rect> boundingboxs;//每个人的boundingbox集合
vector<Point> points;//录制每一帧人的关键点集合
vector<Point> points1;//不录制的时候每一帧人的关键点集合
WinsockMatTransmissionClient socketMat;
bool beginStoreMedia=0;
bool drawBbox=0;
bool replayNextMedia=0;
bool nowPlaying=0;
bool nowPlayingMD=0;

int maxindex;
Mythread::Mythread()
{

}
//run receive string thread
void Mythread::run ()
{
    string str;
    int type=0;
    //发射一个信号，这样主线程就可以安全的对界面进行修改
    //std::cerr<<qPrintable("BB");
    while(1){
         if(socketMat.receiveString(str)>0){//收到字符串
             //std::cerr<<qPrintable(QString::fromStdString(str));
             if(str.empty()){
                 //Sleep(30);
             }else if(str=="zero"){//收到zero表示
                 drawBbox=0;
                 std::cerr<<qPrintable(QString::fromStdString("zero"));
                 emit play2(QString("begin drawnothing"));//转到主线程play2，什么都不画，直接在窗口2放摄像头里的内容
             }else{
                 type=ExplainString(str);//解析字符串
                 if(type==1){//类型1是框（多人时）
                     drawBbox=1;//画框标记
                     emit play2(QString("begin drawbox"));//转到主线程play2，在窗口2画框在显示
                 }else if(type==3){//类型3是points，要录制，表示动作开始后的点
                     drawBbox=0;//不用画框
                     beginStoreMedia=1;//开始录制
                     emit play2(QString("begin drawpoints"));//转到主线程play2，在窗口2画点在显示，并录制
                 }else if(type==4){//类型4是points1，不需录制，表示单人时候窗口2显示的点（动作未开始点）
                     drawBbox=0;//不用画框
                     beginStoreMedia=0;//不用录制
                     emit play2(QString("begin drawpoints1"));//转到主线程play2，在窗口2画点在显示
                 }else if(type==2){//类型2是分数

                     beginStoreMedia=0;//停止录制
                     replayNextMedia=1;//需要重放下一个视频
                     std::cerr<<qPrintable(QString::fromStdString("begin recv score[0]:")+QString::number(scores[0]));
                     while(1){

                         if(nowPlaying==0&&nowPlayingMD==0){//正在播放就等待，放完这一次在重放新的
                             replayNextMedia=0;//不需要重放下一个
                             nowPlaying=1;//正在重放
                             nowPlayingMD=1;
                             std::cerr<<qPrintable(QString::fromStdString("begin replay"));

                             emit replay(QString("begin replay"));//转到主线程replay，在窗口3重放录制的视频
                             break;
                         }else{

                         }
                     }
                     std::cerr<<qPrintable(QString::fromStdString("ok ok"));
                 }

             }

         }
         str.clear();
    }
}
void Mythread::SplitString(const string &s, vector<string> &v, const string &c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}
int Mythread::ExplainString(string str){
    vector<string> splitwords;
    SplitString(str, splitwords, ":");
    //std::cerr<<qPrintable(QString::fromStdString("hhhhh"+splitwords[0]));
    if(splitwords[0]=="bboxs"){//接收到boundingbox的格式： bboxs:startX:startY:endX:endY   (:startX:startY:endX:endY有多组)
        //vector<Rect> boundingboxs;
        int index = 1;
        while( index < splitwords.size()){
            int startX = atoi(splitwords[index++].c_str());
            int startY = atoi(splitwords[index++].c_str());
            int endX = atoi(splitwords[index++].c_str());
            int endY = atoi(splitwords[index++].c_str());
            Rect rect(startX, startY, endX-startX, endY-startY);
            boundingboxs.push_back(rect);
        }
        return 1;
    }else if(splitwords[0]=="scores"){//接收到结果信息的格式： scores:score  (:score有多组)
        //std::cerr<<qPrintable(QString::fromStdString(str));

        int index = 1;
        scores.clear();//清除上次的分数
        while( index < splitwords.size()){
            double score = stod(splitwords[index++]);
            scores.push_back(score);
        }
        maxindex=0;
        double maxscore=0.0;
        std::cerr<<qPrintable(QString::number(scores.size()));

        for(int i=0;i<scores.size();++i){
            if(scores[i]>maxscore){
                maxscore=scores[i];
                maxindex=i;
            }
        }
        return 2;
    }else if(splitwords[0]=="points"){//接收到关键点的格式： points:x:y  (:x:y有多组)
        int index = 1;
        while( index < splitwords.size()){
            int x = atoi(splitwords[index++].c_str());
            int y = atoi(splitwords[index++].c_str());
            points.push_back(Point(x, y));
        }
        //std::cerr<<qPrintable(QString::number(points[0].x)+" "+QString::number(points[0].y));
        //mediaPoints.push_back(points);
        return 3;
    }else if(splitwords[0]=="points1"){//接收到关键点的格式： points:x:y  (:x:y有多组)
        int index = 1;
        while( index < splitwords.size()){
            int x = atoi(splitwords[index++].c_str());
            int y = atoi(splitwords[index++].c_str());
            points.push_back(Point(x, y));
        }
        return 4;
    }
    return 0;
}
