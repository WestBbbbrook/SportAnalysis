
#ifndef VIDEO_SYNOPSIS_SYNOPSIS_H
#define VIDEO_SYNOPSIS_SYNOPSIS_H

//#ifndef _DLL_API
//#define _DLL_API _declspec(dllexport)
//#else
//#define _DLL_API _declspec(dllimport)
//#endif

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/flann.hpp>
#include <list>
#include <stdio.h>
#include<algorithm>

using namespace std;
using namespace cv;

/**
*功能：	取前N帧平均法做背景建模
*参数：	videoFilePath	-	视频路径
*		frame_num_used	-	用于建模帧数
*		bgImg			-	背景图像指针地址（输出）
*		size1			-	高斯平滑框水平尺寸
*		size2			-	高斯平滑框垂直尺寸
*		sigma1			-	高斯平滑框水平sigma
*		sigma2			-	高斯平滑框垂直sigma
*/
void bgModeling(const char * videoFilePath, const char* outBgImgPath, const int frame_num_used, IplImage ** bgImg, \
	int& fps, int& frame_number, CvSize& size, int& code, const int size1 = 5, \
	const int size2 = 5, const int sigma1 = 4, const int sigma2 = 4);


/**
*保存团块的基本数据单元
*/
struct tube {
	//functions
	tube(CvRect rect, int t, IplImage * img); //构造函数
	~tube(); //析构函数

	//variables
	CvRect position; //团块在源图中位置
	int t_sec; //所在帧时间
	IplImage * target; //图像

};


/**
*功能：	检测并跟踪运动前景团块tube，将tubes保存按级联链表结构存储，可以选择保存数据结果
*参数：	videoFilePath	-	视频文件路径 + 文件名
*		bgImg			-	背景灰度图像
*		database		-	级联链表数据的引用（输出）
*		threshold		-	前景和背景分割的阈值
*		min_area		-	检测到的连通区域最小面积
*		obj_num			-	估算视频中待检测目标的个数
*		extend_factor	-	检测框扩展因子（四周扩展 extend_factor * width）
*		category_factor	-	重合度因子（重合度超过因子数则归类为同一跟踪目标）
*		save_mode		-	是否保存跟踪结果图像(保存结果需要在当前路径建立文件夹“DB”)
*/
void buildTrackDB(const char * videoFilePath, const IplImage * bgImg, list< list<tube *> > & database, \
	const int threshold = 100, const int min_area = 900, const int obj_num = 30, \
	const float extend_factor = 0.2, const float category_factor = 0.5, const bool save_mode = false);

void detectEdge_GMM(IplImage* frame, Ptr<BackgroundSubtractorMOG2>& gmm, Mat& edges, \
	int canny_thres_1 = 20, int canny_thres_2 = 190, int canny_apperture = 3);

//void detectEdge_KNN(IplImage* frame, Ptr<BackgroundSubtractorKNN>& knn, Mat& edges, \
	int canny_thres_1 = 1, int canny_thres_2 = 1, int canny_apperture = 5);


void detectEdge_KNN2(Mat frame, Mat bgframe, Mat& edges, \
	int canny_thres_1 = 1, int canny_thres_2 = 1, int canny_apperture = 5);

void buildTrackDB_GMM(const char * videoFilePath, const IplImage * bgImg, list< list<tube *> > & database, \
	const int threshold = 100, const int min_area = 900, const int obj_num = 30, \
	const float extend_factor = 0.2, const float category_factor = 0.5, const bool save_mode = false);


/**
*功能：	融合跟踪数据库形成视频摘要
*参数：	database		-	级联链表数据的引用（输入）
*		videoFilePath	-	输出摘要视频文件路径 + 文件名（例如“E:/synopsis.avi”）
*/
void mergeDB(list< list<tube *> > & database, const char * videoFilePath, const int fps, const CvSize size, const IplImage * bgImg);


/**
*功能： 释放database占据的内存空间
*参数： database	-	级联链表数据的引用
*/
void freeDB(list< list<tube *> > & database);


void reArrangeDB(list< list<tube *> > & database, const char * videoFilePath);

Mat MoveDetect(Mat frame1, Mat frame2);

#endif //VIDEO_SYNOPSIS_SYNOPSIS_H
