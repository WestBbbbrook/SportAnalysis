#include "replaymdthread.h"

int label9width;
int label9height;

ReplayMDThread::ReplayMDThread()
{

}
void ReplayMDThread::run (){
    //一直重放
    while(1){
        if(replayNextMedia==0){//不用重放新的视频
            //repalyDelayMD=cbsize*50/localAllPoint[maxindex].size();
//            std::cerr<<qPrintable(QString::fromStdString("cameraCopy.size():")+" , "+QString::number(cameraCopy.size()));
//            std::cerr<<qPrintable(QString::fromStdString("localAllPoint[maxindex].size():")+" , "+QString::number(localAllPoint[maxindex].size()));
//            std::cerr<<qPrintable(QString::fromStdString("repalyDelayMD:")+" , "+QString::number(repalyDelayMD));
//            std::cerr<<qPrintable(QString::fromStdString("label9width:")+" , "+QString::number(label9width));
//            std::cerr<<qPrintable(QString::fromStdString("label9height:")+" , "+QString::number(label9height));
            for(int i=0;i<localAllPoint[maxindex].size();++i){
                  // std::cerr<<qPrintable(QString::fromStdString("replayingMD:")+" , "+QString::number(localAllPoint[maxindex].size()));
                        //image_md=Mat(Size(label9width,label9height),CV_8UC3,Scalar(255, 255, 255));
                        //image_md=Mat(Size(430,400),CV_8UC3,Scalar(255, 255, 255));//可以用
                        image_md=Mat(Size(262,262),CV_8UC3,Scalar(255, 255, 255));//修改图片大小***
                        drawline(image_md,localAllPoint[maxindex][i][0],localAllPoint[maxindex][i][1],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][2],localAllPoint[maxindex][i][1],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][8],localAllPoint[maxindex][i][1],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][11],localAllPoint[maxindex][i][1],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][5],localAllPoint[maxindex][i][1],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][2],localAllPoint[maxindex][i][3],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][4],localAllPoint[maxindex][i][3],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][5],localAllPoint[maxindex][i][6],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][7],localAllPoint[maxindex][i][6],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][8],localAllPoint[maxindex][i][9],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][10],localAllPoint[maxindex][i][9],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][11],localAllPoint[maxindex][i][12],Scalar(255, 144, 30));
                        drawline(image_md,localAllPoint[maxindex][i][13],localAllPoint[maxindex][i][12],Scalar(255, 144, 30));
                       //cv::resize(image,image,Size(640,480));
                        for(int j=0;j<14;++j){
                            if(localAllPoint[maxindex][i][j].x!=0||localAllPoint[maxindex][i][j].y!=0){
                                circle(image_md, localAllPoint[maxindex][i][j], 7, cv::Scalar(255, 0, 0),-1);//在图像中画出特征点，2是圆的半径
                            }
                        }
                        imshow("as",image_md);
                        img_md =cvMat2QImage(image_md);
                        qsleep(repalyDelayMD);

                        emit replayMD(img_md);
            }
        }else if(replayNextMedia==1){//在重播结束后发现有播放下一个视频的信号，推出死循环
            std::cerr<<qPrintable(QString::fromStdString("replay over"));
            break;
        }
    }
    nowPlayingMD=0; //没有正在播放的视频
}

void ReplayMDThread::qsleep(unsigned int msec){
    QTime reachTime=QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime()<reachTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
}
void ReplayMDThread::drawline(Mat &img,Point a, Point b,Scalar color){
    if(a.x!=0 && a.y!=0 && b.x !=0 && b.y !=0){
        cv::line(img,a,b,color,6);
    }
}
QImage ReplayMDThread::cvMat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
     //   qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
   //     qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

\
