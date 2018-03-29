#include "synopsis.h"
//#include "video_synopsis.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{

	//const char file_path[] = "C:/Users/chenteng/Desktop/VideoTest.avi"; 
	const char file_path[]="E:/Qt/car.mp4";
	const char file_bgimage_path[] = "C:/Users/chenteng/Desktop/bgColorImg.jpg"; //"C:/Users/chenteng/Desktop/bgColorImg.jpg";
	//ocl::setUseOpenCL(true);
	//int fps, frame_number;
	//CvSize size;
	IplImage* bgImage = NULL;
	//const int frame_num_used = 500;
	list< list<tube *> > database;
	//int code = (int)CV_FOURCC('d', 'i', 'v', '3');//MPEG-4.3 codec
	//bgModeling(file_path, file_out_path, frame_num_used, &bgImage, fps, frame_number, size, code);
	bgImage = cvLoadImage(file_bgimage_path, 1);
	//code = (int)CV_FOURCC('x', 'v', 'i', 'd');
	cout << 50;
	int select;
	cout << "input 0 is open camera , input 1 is open local video.."<<endl;
	cin >> select;
	CvCapture *capture;
	if (select == 0){
		capture = cvCreateCameraCapture(0);//打开本地视频
	}
	else if(select==1){
		 capture = cvCaptureFromFile(file_path);
	}
	buildTrackDB_GMM(capture, bgImage, database);


	getchar();
	return 0;
}