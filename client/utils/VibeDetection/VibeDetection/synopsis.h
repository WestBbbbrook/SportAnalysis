//
// Created by xin on 18-1-19.
//

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
*���ܣ�	ȡǰN֡ƽ������������ģ
*������	videoFilePath	-	��Ƶ·��
*		frame_num_used	-	���ڽ�ģ֡��
*		bgImg			-	����ͼ��ָ���ַ�������
*		size1			-	��˹ƽ����ˮƽ�ߴ�
*		size2			-	��˹ƽ����ֱ�ߴ�
*		sigma1			-	��˹ƽ����ˮƽsigma
*		sigma2			-	��˹ƽ����ֱsigma
*/
void bgModeling(const char * videoFilePath, const char* outFilePath, const int frame_num_used, IplImage ** bgImg, \
	int& fps, int& frame_number, CvSize& size, int& code, const int size1 = 5, \
	const int size2 = 5, const int sigma1 = 4, const int sigma2 = 4);


/**
*�����ſ�Ļ������ݵ�Ԫ
*/
struct tube {
	//functions
	tube(CvRect rect, int t, IplImage * img); //���캯��
	~tube(); //��������

	//variables
	CvRect position; //�ſ���Դͼ��λ��
	int t_sec; //����֡ʱ��
	IplImage * target; //ͼ��

};


/**
*���ܣ�	��Ⲣ�����˶�ǰ���ſ�tube����tubes���水��������ṹ�洢������ѡ�񱣴����ݽ��
*������	videoFilePath	-	��Ƶ�ļ�·�� + �ļ���
*		bgImg			-	�����Ҷ�ͼ��
*		database		-	�����������ݵ����ã������
*		threshold		-	ǰ���ͱ����ָ����ֵ
*		min_area		-	��⵽����ͨ������С���
*		obj_num			-	������Ƶ�д����Ŀ��ĸ���
*		extend_factor	-	������չ���ӣ�������չ extend_factor * width��
*		category_factor	-	�غ϶����ӣ��غ϶ȳ��������������Ϊͬһ����Ŀ�꣩
*		save_mode		-	�Ƿ񱣴���ٽ��ͼ��(��������Ҫ�ڵ�ǰ·�������ļ��С�DB��)
*/
void buildTrackDB(CvCapture * capture, const IplImage * bgImg, list< list<tube *> > & database, \
	const int threshold = 100, const int min_area = 900, const int obj_num = 30, \
	const float extend_factor = 0.2, const float category_factor = 0.5, const bool save_mode = false);

void detectEdge_GMM(IplImage* frame, Ptr<BackgroundSubtractorMOG2>& gmm, Mat& edges, \
	int canny_thres_1 = 20, int canny_thres_2 = 190, int canny_apperture = 3);

/*void detectEdge_KNN(IplImage* frame, Ptr<BackgroundSubtractorKNN>& knn, Mat& edges, \
int canny_thres_1 = 1, int canny_thres_2 = 1, int canny_apperture = 5);*/


void detectEdge_KNN2(Mat frame, Mat bgframe, Mat& edges, \
	int canny_thres_1 = 1, int canny_thres_2 = 1, int canny_apperture = 5);

void buildTrackDB_GMM(CvCapture * capture, const IplImage * bgImg, list< list<tube *> > & database, \
	const int threshold = 100, const int min_area = 900, const int obj_num = 30, \
	const float extend_factor = 0.2, const float category_factor = 0.5, const bool save_mode = false);


/**
*���ܣ�	�ںϸ������ݿ��γ���ƵժҪ
*������	database		-	�����������ݵ����ã����룩
*		videoFilePath	-	���ժҪ��Ƶ�ļ�·�� + �ļ��������硰E:/synopsis.avi����
*/
void mergeDB(list< list<tube *> > & database, const char * videoFilePath, const int fps, const CvSize size, const IplImage * bgImg);


/**
*���ܣ� �ͷ�databaseռ�ݵ��ڴ�ռ�
*������ database	-	�����������ݵ�����
*/
void freeDB(list< list<tube *> > & database);


void reArrangeDB(list< list<tube *> > & database, const char * videoFilePath);

Mat MoveDetect(Mat frame1, Mat frame2);

#endif //VIDEO_SYNOPSIS_SYNOPSIS_H
