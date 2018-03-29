#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>
#include "QWidget"
#include "QBoxLayout"
int replayDelay=50;//可以自己设置
int repalyDelayMD;
int notinitscore=0;
int GME_flag=0;
int GME_count=0;
int vibe_flag_ct=0;
int Vibe_count=0;
vector<Mat> cameraBuffer;//存储视频
vector<Mat> cameraCopy;//存储视频
vector<vector<vector<Point> > >localAllPoint;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    label9width=ui->label_9->width();
    label9height=ui->label_9->height();

    if(!notinitscore){
        drawScore(0);
        notinitscore=1;
    }

    QWidget *center = new QWidget(this);
    QGridLayout *layout = new QGridLayout(center);
   // layout->setMargin(150);
    layout->addWidget(ui->widget_title,0,0,1,2);
    layout->addWidget(ui->widget_play,1,0,1,1);
    layout->addWidget(ui->widget_replay,1,1,1,1);
    layout->addWidget(ui->widget_watch,2,0,1,1);
    layout->addWidget(ui->widget_show,2,1,1,1);
    layout->setRowStretch(0,1);
    layout->setRowStretch(1,3);
    layout->setRowStretch(2,3);
    layout->setColumnStretch(0,1);
    layout->setColumnStretch(1,1);

    center->setLayout(layout);
    setCentralWidget(center);

    float w_label_9=ui->label_9->width();
    std::cerr<<qPrintable(QString::number(w_label_9));
    //建立socket连接socketMat，端口6666，向服务器传输视频，返回bbox或者关键点信息
    //if (socketMat.socketConnect(LOCALIP, 6666) < 0)
    if(socketMat.socketConnect(SERVERIP, 6666) < 0)
    {
        std::cerr<<qPrintable("6666 connect  fail ");
    }
    //建立socket连接socketCoordinate，端口7777，触发鼠标点击事件时向服务器传输鼠标坐标，最大匹配跟踪人
    if(socketCoordinate.socketConnect(SERVERIP, 7777) < 0)
   // if (socketCoordinate.socketConnect(LOCALIP, 7777) < 0)
    {
        std::cerr<<qPrintable("7777 connect  fail ");
    }

   //dsize是要传输的大小
    dsize = Size(TRANS_WIDTH,TRANS_HEIGHT);
//    //本地视频文件
//    capture.open("D:/Projects/opencv/ticao.mp4");
//    mediaRate=capture.get(CV_CAP_PROP_FPS);
//    mediaDelay=1000/mediaRate;

    //打开摄像头,0是设备号(电脑摄像头),1就是外接的第一个摄像头
    captureCamera.open(0);
    //延迟，每隔30ms摄像头刷新一帧
    timer1 = new QTimer(this);
    timer1->setInterval(30);
    connect(timer1,SIGNAL(timeout()),this,SLOT(nextCamereFrame()));
    timer1->start();

    //开启子线程实时接收 关键点、bbox、分数等信息
    receriveThread.start();
    connect(&receriveThread,SIGNAL(play2(QString)),this, SLOT(play2(QString)));//关联信号，signal是子线程的，slot是主线程的
    //connect(&receriveThread, SIGNAL(replay(QString)),this, SLOT(replay(QString)));
    connect(&receriveThread, SIGNAL(replay(QString)),this, SLOT(replayStart(QString)));

    //开启子线程读取本地
    readFileThread.start();
}
//读取camera并一帧一帧发送
void MainWindow::nextCamereFrame(){
     captureCamera.read(cframe);
     //cv::cvtColor(cframe,cframe,CV_BGR2RGB);
     //QImage img = QImage((const unsigned char*)(cframe.data),cframe.cols,cframe.rows,QImage::Format_RGB888);

     //std::cerr<<qPrintable(QString::number(vibe_ct));
     QImage img=cvMat2QImage(cframe);
     ui->label_play->setPixmap(QPixmap::fromImage(img));
     ui->label_play->setScaledContents(true);
     ui->label_play->resize(ui->widget->size());
     ui->label_play->show();//窗口1显示摄像头
     if(vibe_flag_ct<3){
         int vibe_ct=vibe(cframe);
         if(vibe_ct>VIBE_Threshold){
             vibe_flag_ct++;
             std::cerr<<qPrintable(QString::fromStdString("vibe_flag_ct=:")+QString::number(vibe_flag_ct));

         }else{
             vibe_flag_ct=0;
         }
         std::cerr<<qPrintable(QString::fromStdString("vibe_ct=:")+QString::number(vibe_ct));

         return;
     }
     if(GME_flag==0){
         double GME=0.0;
         GME=count_GM(cframe);
         //std::cerr<<qPrintable(QString::number(GME));
         if(GME>GME_Threshold){
             std::cerr<<qPrintable(QString::number(GME));
             std::cerr<<qPrintable(QString::fromStdString(">GME_Threshold"));
             GME_flag=1;
             return;
         }else{
             std::cerr<<qPrintable(QString::number(GME));
             std::cerr<<qPrintable(QString::fromStdString("<GME_Threshold"));
         }
     }



     cv::resize(cframe, cframe,dsize);//把camera帧改成服务端需要的大小dsize存在cframe
     mframe=cframe(Rect(0,0,TRANS_WIDTH,TRANS_HEIGHT));//截取cframe一部分放到mframe在窗口2显示用

     if(framenum++%4==0){//跳帧发送
         cv::resize(mframe, mframe,dsize);//把截取的帧mframe改成服务端需要的大小dsize存在mframe
         socketMat.transmit(mframe);//发送
         nowframe=mframe.clone();//把mframe赋给nowframe，为窗口1正在显示的帧
        // std::cerr<<qPrintable(QString::fromStdString("send MAT"));
     }
}
//窗口2显示
void MainWindow::play2(QString sMessage){
    playframe=nowframe.clone();//把刚才显示的帧赋给playframe，窗口2显示，防止绘制（可能慢）的时候视频帧被更改
    //cv::cvtColor(playframe,playframe,CV_BGR2RGB);
    //画bbox，每一帧绘制完bbox再显示
    if(drawBbox){//如果画框
        for(int i=0;i<boundingboxs.size();++i){//所有框都画
            rectangle(playframe,boundingboxs[i],Scalar(0,0,255));
        }
        boundingboxs.clear();//绘制完就清除
    }else{//不画框就看有没有点
        if(!points.empty()){
            //先画线
            drawline(playframe,points[0],points[1],Scalar(255, 144, 30));
            drawline(playframe,points[2],points[1],Scalar(255, 144, 30));
            drawline(playframe,points[8],points[1],Scalar(255, 144, 30));
            drawline(playframe,points[11],points[1],Scalar(255, 144, 30));
            drawline(playframe,points[5],points[1],Scalar(255, 144, 30));
            drawline(playframe,points[2],points[3],Scalar(255, 144, 30));
            drawline(playframe,points[4],points[3],Scalar(255, 144, 30));
            drawline(playframe,points[5],points[6],Scalar(255, 144, 30));
            drawline(playframe,points[7],points[6],Scalar(255, 144, 30));
            drawline(playframe,points[8],points[9],Scalar(255, 144, 30));
            drawline(playframe,points[10],points[9],Scalar(255, 144, 30));
            drawline(playframe,points[11],points[12],Scalar(255, 144, 30));
            drawline(playframe,points[13],points[12],Scalar(255, 144, 30));
            for(int j=0;j<14;++j){//画点
                if(points[j].x!=0||points[j].y!=0){
                    circle(playframe, points[j], 7, cv::Scalar(255, 0, 0),-1);//在图像中画出特征点，2是圆的半径
                }
            }
            points.clear();
            if(beginStoreMedia){//如果画的是points就录制，是points1就不录制
                cameraBuffer.push_back(playframe.clone());//录视频
            }
        }
    }
    //cv::cvtColor(playframe,playframe,CV_BGR2RGB);
    //QImage img2 = QImage((const unsigned char*)(mframe.data),mframe.cols,mframe.rows,QImage::Format_RGB888);
    QImage img2=cvMat2QImage(playframe);
    ui->label_replay->setPixmap(QPixmap::fromImage(img2));
    ui->label_replay->setScaledContents(true);
    ui->label_replay->resize(ui->widget_3->size());
    ui->label_replay->show();//显示
}
void MainWindow::replayStart(QString sMessage){
    drawScore(1);//先画分数
//    ui->label_5->setText(QString::number(scores[0]));
//    ui->label_6->setText(QString::number(scores[1]));
    //拷贝录制的视频，因为重放时候还要录制新的
    for(int i = 0; i <cameraBuffer.size(); ++i){
        if(i<cameraBuffer.size()){
            cameraCopy.push_back(cameraBuffer[i].clone());
        }
    }
    repalyDelayMD=cameraCopy.size()*replayDelay/localAllPoint[maxindex].size();
    cameraBuffer.clear();
    std::cerr<<qPrintable(QString::fromStdString("repalyDelayMD:")+QString::number(repalyDelayMD));

    std::cerr<<qPrintable(QString::fromStdString("replaying:")+QString::number(maxindex));
    //一直重放

    rt.start();
    rmdt.start();
    connect(&rt, SIGNAL(replayC(QImage)),this, SLOT(replayC(QImage)));
    connect(&rmdt, SIGNAL(replayMD(QImage)),this, SLOT(replayMD(QImage)));
    std::cerr<<qPrintable(QString::fromStdString("KKKKK"));
}
void MainWindow::replayC(QImage img){
    ui->label_watch->setPixmap(QPixmap::fromImage(img));
    ui->label_watch->setScaledContents(true);
    ui->label_watch->resize(ui->widget_2->size());
    ui->label_watch->show();//显示在窗口3
}

void MainWindow::replayMD(QImage img_md){

    ui->label_9->setPixmap(QPixmap::fromImage(img_md));
    ui->label_9->setScaledContents(true);
    ui->label_9->resize(ui->widget_5->size());
    ui->label_9->show();//显示在窗口3
    //std::cerr<<qPrintable(QString::fromStdString("JJJJJ"));

}
////画线，如果某个点坐标是0，0就不画
void MainWindow::drawline(Mat &img,Point a, Point b,Scalar color){
    if(a.x!=0 && a.y!=0 && b.x !=0 && b.y !=0){
        cv::line(img,a,b,color,6);
    }
}
//延迟函数
void MainWindow::qsleep(unsigned int msec){
    QTime reachTime=QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime()<reachTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
}
//把MAT类型转化成QImage
QImage MainWindow::cvMat2QImage(const cv::Mat& mat)
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

//鼠标点击事件，点击后mythreadcoordinate线程向服务端传输 鼠标坐标
void MainWindow::mousePressEvent(QMouseEvent *e){
    mouseX=(e->x()-ui->widget_replay->x());
    mouseY=(e->y()-ui->widget_replay->y()-ui->widget_3->y());
    std::cerr<<qPrintable(QString :: number(mouseX) +", "+QString::number(mouseY));
    ThreadCoordinate.start();
}
MainWindow::~MainWindow()
{
    delete ui;
}

Mat MainWindow::drawScore(bool drawscore){
    Mat zft=Mat(Size(560,315),CV_8UC3,Scalar(255, 255, 255));
    line(zft,Point(400,0),Point(400,315),Scalar(0, 0, 0),2);

    line(zft,Point(50,50),Point(50,250),Scalar(0, 0, 255),2);
    line(zft,Point(50,250),Point(350,250),Scalar(0, 0, 255),2);

//    line(zft,Point(50,50),Point(350,50),Scalar(0, 0, 255),1);
//    line(zft,Point(50,70),Point(350,70),Scalar(0, 0, 255),1);
//    line(zft,Point(50,90),Point(350,90),Scalar(0, 0, 255),1);
//    line(zft,Point(50,110),Point(350,110),Scalar(0, 0, 255),1);
//    line(zft,Point(50,130),Point(350,130),Scalar(0, 0, 255),1);
//    line(zft,Point(50,150),Point(350,150),Scalar(0, 0, 255),1);
//    line(zft,Point(50,170),Point(350,170),Scalar(0, 0, 255),1);
//    line(zft,Point(50,190),Point(350,190),Scalar(0, 0, 255),1);
//    line(zft,Point(50,210),Point(350,210),Scalar(0, 0, 255),1);
//    line(zft,Point(50,230),Point(350,230),Scalar(0, 0, 255),1);

    if(drawscore&&(scores.size()!=0)){
        line(zft,Point(50,50),Point(350,50),Scalar(0, 0, 255),1);
        line(zft,Point(50,70),Point(350,70),Scalar(0, 0, 255),1);
        line(zft,Point(50,90),Point(350,90),Scalar(0, 0, 255),1);
        line(zft,Point(50,110),Point(350,110),Scalar(0, 0, 255),1);
        line(zft,Point(50,130),Point(350,130),Scalar(0, 0, 255),1);
        line(zft,Point(50,150),Point(350,150),Scalar(0, 0, 255),1);
        line(zft,Point(50,170),Point(350,170),Scalar(0, 0, 255),1);
        line(zft,Point(50,190),Point(350,190),Scalar(0, 0, 255),1);
        line(zft,Point(50,210),Point(350,210),Scalar(0, 0, 255),1);
        line(zft,Point(50,230),Point(350,230),Scalar(0, 0, 255),1);
        rectangle(zft,Rect(75, 250-200*scores[0], 50, 200*scores[0]),Scalar(0,0,255),-1);
        rectangle(zft,Rect(150, 250-200*scores[1], 50, 200*scores[1]),Scalar(0,255,0),-1);
//        rectangle(zft,Rect(225, 250-200*scores[0], 50, 200*scores[0]),Scalar(255,0,0),-1);
//        rectangle(zft,Rect(300, 250-200*scores[1], 50, 200*scores[1]),Scalar(0,255,255),-1);
//                rectangle(zft,Rect(75, 250-200*0.44, 50, 200*0.44),Scalar(0,0,255),-1);
//                rectangle(zft,Rect(150, 250-200*0.44, 50, 200*0.44),Scalar(0,255,0),-1);
//                rectangle(zft,Rect(225, 250-200*0.44, 50, 200*0.44),Scalar(255,0,0),-1);
//                rectangle(zft,Rect(300, 250-200*0.44, 50, 200*0.44),Scalar(0,255,255),-1);
    }

    rectangle(zft,Rect(400, 50, 40, 20),Scalar(0,0,255),-1);
    //putText(zft, "zhengshou", Point(440,70), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255,0 ), 1);
    rectangle(zft,Rect(400, 110, 40, 20),Scalar(0,255,0),-1);
    //putText(zft, "fanshou", Point(440,130), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255,0 ), 1);
    rectangle(zft,Rect(400, 170, 40, 20),Scalar(255,0,0),-1);
   // putText(zft, "反手击球", Point(440,190), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255,0 ), 1);
    rectangle(zft,Rect(400, 230, 40, 20),Scalar(0,255,255),-1);
    //putText(zft, "正手击球", Point(440,250), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255,0 ), 1);

    QImage img=cvMat2QImage(zft);
    ui->label_10->setPixmap(QPixmap::fromImage(img));
    ui->label_10->setScaledContents(true);
    ui->label_10->resize(ui->widget_6->size());
    ui->label_10->show();
    return zft;
}
Mat& MainWindow::MyGammaCorrection(Mat& I, float fGamma){
    CV_Assert(I.data);
    // accept only char type matrices
    CV_Assert(I.depth() != sizeof(uchar));
    // build look up table
    unsigned char lut[256];
    for (int i = 0; i < 256; i++){
        lut[i] = pow((float)(i / 255.0), fGamma) * 255.0;
    }
    const int channels = I.channels();
    switch (channels){
    case 1:{
              MatIterator_<uchar> it, end;
              for (it = I.begin<uchar>(), end = I.end<uchar>(); it != end; it++)
                  //*it = pow((float)(((*it))/255.0), fGamma) * 255.0;
                  *it = lut[(*it)];
              break;
    }
    case 3:{
              MatIterator_<Vec3b> it, end;
              for (it = I.begin<Vec3b>(), end = I.end<Vec3b>(); it != end; it++){
                  //(*it)[0] = pow((float)(((*it)[0])/255.0), fGamma) * 255.0;
                  //(*it)[1] = pow((float)(((*it)[1])/255.0), fGamma) * 255.0;
                  //(*it)[2] = pow((float)(((*it)[2])/255.0), fGamma) * 255.0;
                  (*it)[0] = lut[((*it)[0])];
                  (*it)[1] = lut[((*it)[1])];
                  (*it)[2] = lut[((*it)[2])];
              }
              break;
    }
    }
    return I;
}
bool MainWindow::cvMatEQ(const cv::Mat& data1, const cv::Mat& data2)
{
    bool success = true;
    // check if is multi dimensional
    if (data1.dims > 2 || data2.dims > 2)
    {
        if (data1.dims != data2.dims || data1.type() != data2.type())
        {
            return false;
        }
        for (int dim = 0; dim < data1.dims; dim++){
            if (data1.size[dim] != data2.size[dim]){
                return false;
            }
        }
    }
    else
    {
        if (data1.size() != data2.size() || data1.channels() != data2.channels() || data1.type() != data2.type()){
            return false;
        }
    }
    int nrOfElements = data1.total()*data1.elemSize1();
    //bytewise comparison of data
    int cnt = 0;
    for (cnt = 0; cnt < nrOfElements && success; cnt++)
    {
        if (data1.data[cnt] != data2.data[cnt]){
            success = false;
        }
    }
    return success;
}
double MainWindow::count_GM(Mat oriFrame){
    Mat gammaFrame = MyGammaCorrection(oriFrame, 0.45);

    Mat channels[3];
    split(oriFrame, channels);//分离色彩通道
    //imshow("channels0", channels[0]);
    //imshow("channels1", channels[1]);
    //imshow("channels2", channels[2]);

    Mat mask_neg1;
    Mat mask_0 = cv::Mat::zeros(oriFrame.size(), CV_32F);
    Mat mask_1 = cv::Mat::ones(oriFrame.size(), CV_32F);
    cv::subtract(mask_1, mask_0, mask_neg1);
    //imshow("mask", mask_neg1);
    //imshow("mask0", mask_0);
    //imshow("mask1", mask_1);

    double max_grad_magni = -10000000.0;
    /*
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y, x2y2, grad_magni;

    Sobel(oriFrame, grad_x, oriFrame.depth(), 1, 0);
    convertScaleAbs(grad_x, abs_grad_x);
    imshow("abs_grad_x", abs_grad_x);

    Sobel(oriFrame, grad_y, oriFrame.depth(), 0, 1);
    convertScaleAbs(grad_y, abs_grad_y);
    imshow("abs_grad_y", abs_grad_y);

    pow(abs_grad_x, 2.0f, abs_grad_x);
    pow(abs_grad_y, 2.0f, abs_grad_y);

    add(abs_grad_x, abs_grad_y, x2y2, noArray(), CV_32F);
    imshow("x2y2", x2y2);

    sqrt(x2y2, grad_magni);
    imshow("gradient magnitud", grad_magni);*/
    //if (cvMatEQ(x2y2, x2y2))
    //	cout << "haha";
    Mat mask_res0, mask_res1, mask_res2;
    double GME = 0.0;
    double sum_his = 0.0;
    Mat  tidu, jiaodu;


    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;
    Sobel(channels[2], grad_x, oriFrame.depth(), 1, 0);
    convertScaleAbs(grad_x, abs_grad_x);
    //imshow("abs_grad_x", abs_grad_x);

    Sobel(channels[2], grad_y, oriFrame.depth(), 0, 1);
    convertScaleAbs(grad_y, abs_grad_y);
    grad_x.convertTo(grad_x, CV_32FC3, 1 / 255.0);
    grad_y.convertTo(grad_y, CV_32FC3, 1 / 255.0);
    //cout << grad_x << endl;
    int his[180] = { 0 };
    cartToPolar(grad_x, grad_y, tidu, jiaodu, true);
    for (int col = 0; col < tidu.cols; col++){
        for (int row = 0; row < tidu.rows; row++){
            int td = (int)(*(tidu.data + tidu.step[0] * row + tidu.step[1] * col));
            int jd = (int)(*(jiaodu.data + tidu.step[0] * row + tidu.step[1] * col));
            if (td != 0){
                //cout << k << endl;
                //cout << k << "  " << atan(k) * 180/3.1415926 << endl;
                // his[int(floor(abs(atan((double)(*(mask_res.data + mask_res.step[0] * row + mask_res.step[1] * col))))))]++;
                if (jd >= 180){
                    jd -= 180;
                }
                his[jd]++;
            }
        }
    }
    //cout << tidu<< endl;
    //cout << jiaodu << endl;
    for (int i = 0; i < 180; ++i){
        //cout << i << "  " << his[i] << endl;
        sum_his += his[i];
    }
    double pi = 0.0;
    for (int i = 0; i < 180; ++i){
        pi = his[i] / sum_his;
        GME -= pi*log(pi);
    }

    //cout << GME << endl;
    return GME;
}

int MainWindow::vibe(Mat frame_now){
    const char file_bgimage_path[] = "C:/Users/chenteng/Desktop/bgColorImg.jpg";
    IplImage* bgImg = cvLoadImage(file_bgimage_path, 1);
    //CvCapture *capture= cvCreateCameraCapture(0);
    //buildTrackDB_GMM(capture, bgImage, database);
    //std::cerr<<qPrintable(QString::number(4));

    if (bgImg == NULL) {
        //printf("Fail: NULL background image!\n");
        //std::cerr<<qPrintable(QString::fromStdString("Fail: NULL background image!"));
        return -1;
        }
    std::cerr<<qPrintable(QString::number(5));

   if (bgImg->nChannels != 3) {
        //printf("Fail: Background image should be grayscale!\n");
        //std::cerr<<qPrintable(QString::fromStdString("Fail: Background image should be grayscale!"));
        return -1;
   }
   std::cerr<<qPrintable(QString::number(6));

   int frame_id = 0;
   //IplImage * frame = NULL;

   int find_num = 0;
   list<CvRect> rects; //存放找到的前景块位置
   int last_find_num = 0;
   //printf("Build track database...\n");
   //std::cerr<<qPrintable(QString::fromStdString("Build track database..."));

   Mat frame_mat=frame_now;

   vector<Mat> contours;
   Mat hierarchy;

            //frame_mat = cvarrToMat(tmp,true);
            //cvAbsDiff(bgImg, frame, frame1);

   Mat frame_mat1 = cvarrToMat(bgImg, true);

   Mat edges;
 /*  if (frame == NULL){
        break;
   }*/
            //detectEdge_KNN(frame, m_KNN, edges);
            //detectEdge_KNN2(frame_mat, frame_mat1, edges);
   edges = MoveDetect(frame_mat, frame_mat1);
   //std::cerr<<qPrintable(QString::number(7));

   findContours(edges, contours, hierarchy, CV_RETR_CCOMP, CHAIN_APPROX_NONE);
   last_find_num = find_num;
   find_num = contours.size();

   //imshow("KNN前景", edges);
            /**!!!Be careful with the param below!!!探测数量上限**/
 /* if (find_num > (obj_num * 3) || frame == NULL) {
         continue;
   }*/

  // printf("frame:%d\tfind contour num:%d\n", frame_id, find_num);
 // std::cerr<<qPrintable(QString::fromStdString("findnum:")+QString::number(find_num));
            //contour analysis
   if (contours.size()>0){
        for (int i = 0; i < contours.size(); i++){
             Rect r = boundingRect(contours[i]);
             if (r.height * r.width > 2000) {
                     rects.push_back(r);
             }
                    //centers.push_back((r.br()+r.tl())*0.5);
        }
    }
   // cout << "contours.size" << contours.size();

    //cvShowImage("frame", frame);
            //cvShowImage("fgImg", fgImg);
    cvWaitKey(10);
    frame_id += 1;
    rects.clear();
    return contours.size();
}
Mat MainWindow::MoveDetect(Mat frame1, Mat frame2) {
    Mat result = frame2.clone();
    Mat gray1, gray2;
    cvtColor(frame1, gray1, CV_BGR2GRAY);
    cvtColor(frame2, gray2, CV_BGR2GRAY);

    Mat diff;
    absdiff(gray1, gray2, diff);
    //imshow("absdiss", diff);
    threshold(diff, diff, 25, 255, CV_THRESH_BINARY);
    imshow("threshold", diff);

    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat element2 = getStructuringElement(MORPH_RECT, Size(19, 19));
    erode(diff, diff, element);
    //imshow("erode", dst);
    medianBlur(diff, diff, 3);
    imshow("medianBlur", diff);
    dilate(diff, diff, element2);
    //imshow("dilate", diff);
    result = diff;
    return result;

}
