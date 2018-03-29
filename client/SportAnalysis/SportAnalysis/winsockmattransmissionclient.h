//  基于OpenCV和Winsock的图像传输（发送）
#ifndef __WINSOCKMATTRANSMISSIONCLIENT_H__
#define __WINSOCKMATTRANSMISSIONCLIENT_H__
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include <stdio.h>
#include <Winsock2.h>
#include <thread>
#include <string>

#pragma comment(lib,"WS2_32.lib")

//待传输图像默认大小为 640*480，可修改
#define IMG_WIDTH 656//560	// 需传输图像的宽
#define IMG_HEIGHT 368//315	// 需传输图像的高
//默认格式为CV_8UC3
#define BUFFER_SIZE IMG_WIDTH*IMG_HEIGHT*3/32
struct sentbuf
{
    char buf[BUFFER_SIZE];
    int flag;
};
class WinsockMatTransmissionClient
{
public:
    WinsockMatTransmissionClient(void);
    ~WinsockMatTransmissionClient(void);
private:
    SOCKET sockClient;
    struct sentbuf data;
public:

    // 打开socket连接
    // params :	IP		服务器的ip地址
    //			PORT	传输端口
    // return : -1		连接失败
    //			1		连接成功
    int socketConnect(const char* IP, int PORT);

    // 传输图像
    // params : image 待传输图像
    // return : -1		传输失败
    //			1		传输成功
    int transmit(cv::Mat image);

    int sendDouble(double num);
    int receiveDouble(double& num);
    int receiveString(std::string &str);
    int sendString(std::string str);
    // 断开socket连接
    void socketDisconnect(void);

};

#endif
