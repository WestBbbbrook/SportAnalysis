#include "replaythread.h"
ReplayThread::ReplayThread()
{

}
void ReplayThread::run (){
    //一直重放
    while(1){
        //std::cerr<<qPrintable(QString::fromStdString("123"));
        if(replayNextMedia==0){//不用重放新的视频
            //std::cerr<<qPrintable(QString::fromStdString("456"));
            for(int i=0;i<cameraCopy.size();++i){
                //std::cerr<<qPrintable(QString::fromStdString("replaying:")+QString :: number(cameraCopy.size()));
                image = cameraCopy[i].clone();
                img =cvMat2QImage(image);

                qsleep(50);
                emit replayC(img);
            }
        }else if(replayNextMedia==1){//在重播结束后发现有播放下一个视频的信号，推出死循环
            std::cerr<<qPrintable(QString::fromStdString("replay over"));
            break;
        }
        //std::cerr<<qPrintable(QString::fromStdString("999"));
    }
    cameraCopy.clear(); //绘制结束后清空cameraCopy
    nowPlaying=0; //没有正在播放的视频
}

void ReplayThread::qsleep(unsigned int msec){
    QTime reachTime=QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime()<reachTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
}
QImage ReplayThread::cvMat2QImage(const cv::Mat& mat)
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

void ReplayThread::drawline(Mat &img,Point a, Point b,Scalar color){
    if(a.x!=0 && a.y!=0 && b.x !=0 && b.y !=0){
        cv::line(img,a,b,color,6);
    }
}
